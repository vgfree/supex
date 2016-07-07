/*
 * CopyRight    : DT+
 * Author       : louis.tin
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

#include "major/smart_api.h"
#include "appsrv.h"
#include "json.h"

#include "core/evcs_module.h"
#include "core/evcs_events.h"
#include "core/evcs_kernel.h"
#include "spx_evcs.h"
#include "spx_evcs_module.h"
#include "async_tasks/async_obj.h"
#include "base/free_queue.h"
#include "lua_expand/lj_c_coro.h"
#include "lua_expand/lua_link.h"
#include "lua_expand/lj_http_info.h"
#include "lua_expand/lj_cache.h"
#include "luakv/luakv.h"

EVCS_MODULE_SETUP(kernel, kernel_init, kernel_exit, &g_kernel_evts);
EVCS_MODULE_SETUP(evcs, evcs_init, evcs_exit, &g_evcs_evts);
#include "_ev_coro.h"
#include "evcoro_async_tasks.h"
#include "thread_pool_loop/tlpool.h"


#define MAX_PTHREAD_COUNT 3
#define MAX_UTHREAD_COUNT 5

struct smart_cfg_list g_smart_cfg_list = {};




lua_State *lua_vm_init(void)
{
	int             error = 0;
	lua_State       *L = luaL_newstate();
	assert(L);
	luaopen_base(L);
	luaL_openlibs(L);

	lua_register(L, "supex_http", async_http);
	lua_register(L, "lua_default_switch", lj_evcoro_switch);
	lua_register(L, "search_kvhandle", search_kvhandle);

	/*lua init*/
	{
		int app_lua_get_serv_name(lua_State *L)
		{
			lua_pushstring(L, "appServer");
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
	lua_pushinteger(L, supex_get_default()->scheduler);
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
	tlpool_t *pool = user;
	bool ok = false;
	int idx = tlpool_get_thread_index(pool);
	// 抢占模式
	ok = tlpool_pull(pool, task, TLPOOL_TASK_SEIZE, idx);

	return ok;
}

void *task_handle(struct supex_evcoro *evcoro, int step)
{
	lua_State *L = NULL;
	// 将数据取出
	struct app_msg *p_task = &((struct app_msg *)evcoro->task)[step];
	union virtual_system *p_VMS = &((union virtual_system *)evcoro->VMS)[step];
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
	for (i = 0; i < p_task->vector_size; ++i) {
		lua_pushnumber(L, i+1);
		lua_pushlstring(L, p_task->vector[i].iov_base, p_task->vector[i].iov_len);
		free(p_task->vector[i].iov_base);
		lua_settable(L, -3);
	}
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

	struct evcs_argv_settings sets = {
		.num    = MAX_UTHREAD_COUNT     /*协程数*/
			, .tsz  = sizeof(struct app_msg)
			, .data = data
			, .task_lookup = task_lookup
			, .task_handle = task_handle
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

int main(int argc, char **argv)
{
	if (!kvpool_init()) {
		exit(EXIT_FAILURE);
	}
	create_io();
	// 1. 初始化线程池
	tlpool_t *tlpool = tlpool_init(MAX_PTHREAD_COUNT, 100, sizeof(struct app_msg), NULL);
	int idx;
	for (idx = 0; idx < MAX_PTHREAD_COUNT; idx++) {
		tlpool_bind(tlpool, (void (*)(void *))task_worker, tlpool, idx);
	}
	tlpool_boot(tlpool);

	// 2. 循环接收数据
	struct app_msg recv_msg = {};
	int more = 0;

	char *t1 = malloc(8);
	memcpy(t1, "upstream", 8);
	char *t2 = malloc(10);
	memcpy(t2, "cid 5", 6);
	char *t3 = malloc(20);
	memcpy(t3, "{opt:567}", 10);
	recv_msg.vector_size = 3;
	recv_msg.vector[0].iov_base = t1;
	recv_msg.vector[0].iov_len = 8;
	recv_msg.vector[1].iov_base = t2;
	recv_msg.vector[1].iov_len = 10;
	recv_msg.vector[2].iov_base = t3;
	recv_msg.vector[2].iov_len = 20;
	task_report(tlpool, &recv_msg);
	while (1) {
		if (recv_app_msg(&recv_msg, &more, -1)) {
			task_report(tlpool, &recv_msg);
			printf("在这里\n");
		}
	}
}


/*
 * 数据帧格式
 *
 * Login server 发出的命令
 * 0 status
 * 1 [connected]/[closed]
 * 2 CID
 *
 * 发送给Setting server 的命令
 * 0 setting
 * 1 [status]/[uidmap]/[gidmap]
 * 2 CID
 * 3 [closed]/[uid]/[gid]
 *
 * 普通消息下发
 * 0 downstream
 * 1 [cid]/[uid]/[gid]
 * 2 CID/UID/GID
 *
 * bind 消息下发
 * 0 downstream
 * 1 [cid]
 * 2 CID
 * 3 [bind]
 *
 * bind 消息上行
 * 0 upstream
 * 1 CID
 * 2 [bind]
 * 3 {UID}
 */




