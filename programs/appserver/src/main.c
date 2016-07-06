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

#include "load_cfg.h"
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

EVCS_MODULE_SETUP(kernel, kernel_init, kernel_exit, &g_kernel_evts);
EVCS_MODULE_SETUP(evcs, evcs_init, evcs_exit, &g_evcs_evts);
#include "_ev_coro.h"
#include "evcoro_async_tasks.h"
#include "thread_pool_loop/tlpool.h"


#define MAX_PTHREAD_COUNT 3
#define MAX_UTHREAD_COUNT 5

struct smart_cfg_list g_smart_cfg_list = {};


static int get_uid(lua_State *L)
{
	printf("下发数据: 获取uid\n");

	char *cid_str = lua_tostring(L, -1);

	char *downstream = "downstream";
	char *cid = "cid";
	char *msg = "bind";
	printf("cid = %s, cid_str = %s, msg = %s\n", cid, cid_str, msg);

	struct app_msg send_msg = {};
	send_msg.vector_size = 4;
	send_msg.vector[0].iov_base = downstream;
	send_msg.vector[0].iov_len = strlen(downstream);
	send_msg.vector[1].iov_base = cid;
	send_msg.vector[1].iov_len = strlen(cid);
	send_msg.vector[2].iov_base = cid_str;
	send_msg.vector[2].iov_len = strlen(cid_str);
	send_msg.vector[3].iov_base = msg;
	send_msg.vector[3].iov_len = strlen(msg);
	send_app_msg(&send_msg);

	printf("获取uid完成\n\n");

	return 1;
}

static int set_uidmap(lua_State *L)
{
	printf("下发数据: 绑定uidmap\n");

	int n = lua_gettop(L);
	printf("stack number is %d\n",n);

	lua_pushnumber(L, 1);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_1 = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushnumber(L, 2);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_2 = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushnumber(L, 3);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_3 = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushnumber(L, 4);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_4 = lua_tostring(L, -1);
	lua_pop(L, 1);

	struct app_msg  send_msg = {};
	send_msg.vector_size = 4;
	send_msg.vector[0].iov_base = frame_1;
	send_msg.vector[0].iov_len = strlen(frame_1);
	send_msg.vector[1].iov_base = frame_2;
	send_msg.vector[1].iov_len = strlen(frame_2);
	send_msg.vector[2].iov_base = frame_3;
	send_msg.vector[2].iov_len = strlen(frame_3);
	send_msg.vector[3].iov_base = frame_4;
	send_msg.vector[3].iov_len = strlen(frame_4);
	send_app_msg(&send_msg);
	printf("绑定完成\n\n");

	return 1;
}

static int send_msg(lua_State *L)
{
	printf("下发消息\n");

	int n = lua_gettop(L);

	lua_pushnumber(L, 1);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_1 = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushnumber(L, 2);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_2 = lua_tostring(L, -1);
	lua_pop(L, 1);

	lua_pushnumber(L, 3);
	lua_gettable(L, -2);
	printf("setting = %s\n", lua_tostring(L, -1));
	char *frame_3 = lua_tostring(L, -1);
	lua_pop(L, 1);

	struct app_msg  send_msg = {};
	send_msg.vector_size = 4;
	send_msg.vector[0].iov_base = frame_1;
	send_msg.vector[0].iov_len = strlen(frame_1);
	send_msg.vector[1].iov_base = frame_2;
	send_msg.vector[1].iov_len = strlen(frame_2);
	send_msg.vector[2].iov_base = frame_3;
	send_msg.vector[2].iov_len = strlen(frame_3);
	printf("消息发送完成\n\n");

	return 1;

}

lua_State *lua_vm_init(void)
{
	int             error = 0;
	lua_State       *L = luaL_newstate();
	assert(L);
	luaopen_base(L);
	luaL_openlibs(L);

	lua_register(L, "get_uid", get_uid);
	lua_register(L, "set_uidmap", set_uidmap);
	lua_register(L, "send_msg",  send_msg);


	error = luaL_dofile(L, "lua/core/init.lua");
	if (error) {
		x_printf(E, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		exit(EXIT_FAILURE);
	}

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
		lua_settable(L, -3);
	}
	error = lua_pcall(L, 1, 0, 0);
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

	while (1) {
		if (recv_app_msg(&recv_msg, &more, -1)) {
			task_report(tlpool, &recv_msg);
			printf("在这里\n");
		}
	}
#if 0
	/*******************************************/
	// 执行数据处理
	lua_State *L;
	L = luaL_newstate(); // 打开lua
	luaL_openlibs(L); // 打开标准库

//	luaopen_power(L);

	int status = luaL_loadfile(L, "script.lua");
	if (status) {
		perror("luaL_dofile error");
		exit(1);
	}

	lua_newtable(L);
	int i;
	for (i = 1; i <= recv_msg.vector_size; i++) {
		printf("recv_msg size:%d\t [%d]iov_len:%d\t [%d]iov_base = %s,\n",
				recv_msg.vector_size, i - 1, recv_msg.vector[i - 1].iov_len, i -1 , recv_msg.vector[i - 1].iov_base);
		lua_pushnumber(L, i);
		lua_pushlstring(L, recv_msg.vector[i - 1].iov_base, recv_msg.vector[i - 1].iov_len);
		lua_rawset(L, -3);
	}

	lua_setglobal(L, "msg");

	int result = lua_pcall(L, 0, LUA_MULTRET, 0);
	if (result) {
		fprintf(stdout, "bad, bad script\n");
		exit(1);
	}    /* 获得堆栈顶的值*/
	int sum = lua_tonumber(L, lua_gettop(L));
	if (!sum) {
		fprintf(stdout, "lua_tonumber() failed!\n");
		exit(1);
	}
	fprintf(stdout, "Script returned: %d\n", sum);
	lua_pop(L, 1);
	printf("top = %d\n", lua_gettop(L));
	lua_close(L);
#endif
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




