/*
 * CopyRight    : DT+
 * Author       : huiqi.qian
 * Date         : 06-28-2016
 * Description  : Appserver
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <memory.h>
#include <uuid/uuid.h>
#include <time.h>
#include <pthread.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "json.h"

#include "libkv.h"
#include "libmini.h"
#include "libevcs.h"
#include "comm_api.h"
#include "appsrv_o_wrap.h"

EVCS_MODULE_SETUP(kernel, kernel_init, kernel_exit, &g_kernel_evts);
EVCS_MODULE_SETUP(evcs, spx_evcs_init, spx_evcs_exit, &g_spx_evcs_evts);

#define MAX_PTHREAD_COUNT       3
#define MAX_UTHREAD_COUNT       5

struct smart_cfg_list g_smart_cfg_list = {};

extern int      g_stream_sfd;
extern int      g_status_sfd;
extern int      g_usrapi_sfd;

static int downstream_msg(struct comm_message *msg, int sktfd)
{
	commmsg_sets(msg, sktfd, 0, PUSH_METHOD);

	if (appsrv_o_wrap_send(msg) == -1) {
		x_printf(E, "wrong msg, msg fd:%d.", msg->fd);
	}

	return 0;
}

static int app_lua_send(lua_State *L, int sktfd)
{
	if (lua_istable(L, -1) == 0) {
		lua_pushboolean(L, 0);
		return 1;
	}

	struct comm_message sendmsg = { 0 };
	commmsg_make(&sendmsg, 1024);
	commmsg_sets(&sendmsg, sktfd, 0, PUSH_METHOD);

	int             i;
	int             nub = lua_objlen(L, 1);

	for (i = 1; i <= nub; i++) {
		lua_pushnumber(L, i);
		lua_gettable(L, -2);
		size_t fsize = 0;
		const char *frame = luaL_checklstring(L, -1, &fsize);

		/*send fram*/
		commmsg_frame_set(&sendmsg, i - 1, fsize, (char *)frame);
		sendmsg.package.frames_of_package[0] = sendmsg.package.frames;
		sendmsg.package.packages = 1;

		/*do next*/
		lua_pop(L, 1);
	}

	lua_pushboolean(L, 1);

	downstream_msg(&sendmsg, sktfd);

	commmsg_free(&sendmsg);

	return 1;
}

int app_lua_send_stream(lua_State *L)
{
	return app_lua_send(L, g_stream_sfd);
}

int app_lua_send_usrapi(lua_State *L)
{
	return app_lua_send(L, g_usrapi_sfd);
}

lua_State *lua_vm_init(void)
{
	int             error = 0;
	lua_State       *L = luaL_newstate();

	assert(L);
	luaopen_base(L);
	luaL_openlibs(L);

	lua_register(L, "supex_http", async_http);
	lua_register(L, "lua_default_switch", lj_evcoro_switch);
	lua_register(L, "app_lua_send_stream", app_lua_send_stream);
	lua_register(L, "app_lua_send_usrapi", app_lua_send_usrapi);

	/*lua init*/
	{
		int app_lua_get_serv_name(lua_State *L)
		{
			lua_pushstring(L, "appSrv");
			return 1;
		}

		lua_register(L, "app_lua_get_serv_name", app_lua_get_serv_name);

		error = luaL_dofile(L, "lua/core/init.lua");

		if (error) {
			x_printf(E, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
			exit(EXIT_FAILURE);
		}
	}
	/*app init*/
	error = luaL_dofile(L, "lua/core/start.lua");

	if (error) {
		x_printf(E, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		exit(EXIT_FAILURE);
	}

	lua_getglobal(L, "app_evcoro_init");
	lua_pushinteger(L, (uintptr_t)supex_get_default()->scheduler);
	lua_pushinteger(L, 0);
	error = lua_pcall(L, 2, 0, 0);

	if (error) {
		x_printf(E, "%s", lua_tostring(L, -1));
		lua_pop(L, 1);
		exit(EXIT_FAILURE);
	}

	return L;
}

// 监视是否有数据
static bool task_lookup(void *user, void *task)
{
	tlpool_t        *pool = user;
	bool            ok = false;
	int             idx = tlpool_get_thread_index(pool);

	// 抢占模式
	ok = tlpool_pull(pool, task, TLPOOL_TASK_SEIZE, idx);

	return ok;
}

void *task_handle(struct supex_evcoro *evcoro, int step)
{
	lua_State *L = NULL;
	// 将数据取出
	struct comm_message     *msg = ((struct comm_message **)evcoro->task)[step];
	union virtual_system    *p_VMS = &((union virtual_system *)evcoro->VMS)[step];

	// 判断Lua虚拟机是否存在, 存在则赋给L, 不存在则初始化
	if (p_VMS->L) {
		L = p_VMS->L;
		x_printf(D, "No need to init LUA VM!\n");
	} else {
		L = lua_vm_init();
		evcoro->VMS[step].L = L;
		x_printf(S, "first init LUA VM!\n");
	}

	if (!L) {
		printf("lua vm is NULL\n");
		exit(0);
	}

	/******************************************/

	// 执行数据处理
	int i;
	lua_getglobal(L, "app_call");
	lua_newtable(L);

	for (i = 0; i < commmsg_frame_count(msg); i++) {
		int     fsize = 0;
		char    *frame = commmsg_frame_get(msg, i, &fsize);

		lua_pushnumber(L, i + 1);
		lua_pushlstring(L, frame, fsize);
		lua_settable(L, -3);
	}

	commmsg_free(msg);

	int error = lua_pcall(L, 1, 0, 0);

	if (error) {
		assert(L);
		x_printf(E, "%s", lua_tostring(L, -1));
		lua_pop(L, 1);
	}

	return NULL;
}

void task_worker(void *data)
{
	/*load module*/
	EVCS_MODULE_MOUNT(kernel);
	EVCS_MODULE_ENTRY(kernel, true);

	struct spx_evcs_argv_settings sets = {
		.num    = MAX_UTHREAD_COUNT	/*协程数*/
		, .tsz  = sizeof(struct comm_message *)
		, .data = data
		, .task_lookup= task_lookup
		, .task_handle= task_handle
	};
	EVCS_MODULE_CARRY(evcs, &sets);
	EVCS_MODULE_MOUNT(evcs);
	EVCS_MODULE_ENTRY(evcs, true);

	EVCS_MODULE_START();
}

static bool task_report(void *user, void *task)
{
	tlpool_t        *pool = user;
	bool            ok = false;

	ok = tlpool_push(pool, task, TLPOOL_TASK_SEIZE, 0);
	return ok;
}

static tlpool_t *tlpool = NULL;
static void *_pull_thread(void *usr)
{
	// 2. 循环接收数据
	while (1) {
		struct comm_message *msg = commmsg_make(NULL, DEFAULT_MSG_SIZE);
		appsrv_o_wrap_recv(msg);

#define debug 1
#ifdef debug
		x_printf(D, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
		commmsg_print(msg);
		x_printf(D, "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
#endif
		task_report(tlpool, &msg);
	}

	return NULL;
}

static pthread_t tid1;
void appsrv__work(void)
{
	assert(appsrv_o_wrap_init() == 0);

	/*work push*/
	int err = pthread_create(&tid1, NULL, _pull_thread, NULL);

	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}

	x_printf(I, "appSrv work!\n");
}

void appsrv__wait(void)
{
	/*over*/
	void *status = NULL;

	pthread_join(tid1, status);
}

void appsrv__stop(void)
{
	x_printf(W, "appSrv stop!\n");
	appsrv_o_wrap_exit();
}

int main(int argc, char **argv)
{
	// if (!kvpool_init(kv_create)) {
	//	exit(EXIT_FAILURE);
	// }

	// 1. 初始化线程池
	tlpool = tlpool_init(MAX_PTHREAD_COUNT, 100, sizeof(struct comm_message *), NULL, NULL);
	int idx;

	for (idx = 0; idx < MAX_PTHREAD_COUNT; idx++) {
		tlpool_bind(tlpool, (void (*)(void *))task_worker, tlpool, idx);
	}

	tlpool_boot(tlpool);

	appsrv__work();
	appsrv__wait();
	appsrv__stop();
}

