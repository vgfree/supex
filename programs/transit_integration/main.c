#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "skt.h"

#define SERVER_NAME "AK47"

static lua_State *lua_vm_init(void)
{
	int             error = 0;
	lua_State       *L = luaL_newstate();

	assert(L);
	luaopen_base(L);
	luaL_openlibs(L);
	/*reg func*/
	// lua_register(L, "app_lua_lru_cache_set_value", lrucache_setvalue);
	// lua_register(L, "app_lua_lru_cache_get_value", lrucache_getvalue);
	// lua_register(L, "app_lua_lru_cache_remove_value", lrucache_removevalue);

	/*lua init*/
	{
		int app_lua_get_serv_name(lua_State *L)
		{
			lua_pushstring(L, SERVER_NAME);
			return 1;
		}

		lua_register(L, "app_lua_get_serv_name", app_lua_get_serv_name);

		error = luaL_dofile(L, "init.lua");

		if (error) {
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
			exit(EXIT_FAILURE);
		}
	}
	/*app init*/
	error = luaL_dofile(L, "start.lua");

	if (error) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		exit(EXIT_FAILURE);
	}

	lua_getglobal(L, "app_init");
	lua_pcall(L, 0, 0, 0);
	return L;
}

void *work_task(void *args)
{
	int             error = 0;
	void            *data = NULL;
	void            *g_subscriber = NULL;
	lua_State       *L = lua_vm_init();

	struct skt_device devc = {};

	zmq_Javasrv_init(&g_subscriber);
	assert(g_subscriber != NULL);

	while (1) {
		zmq_srv_fetch(&devc);
		// TODO add
		int ok = zmq_sendiov(g_subscriber, devc.ibuffer, devc.idx, ZMQ_SNDMORE);
		printf("ok = %d\n", ok);

		lua_getglobal(L, "app_call");
		lua_newtable(L);
		int i = 0;
		printf("cnt %d\n", devc.idx);

		for (i = 0; i < devc.idx; i++) {
			data = devc.ibuffer[i].iov_base;
			lua_pushnumber(L, i + 1);
			lua_pushlstring(L, data, devc.ibuffer[i].iov_len);
			lua_settable(L, -3);

			free(data);
		}

		error = lua_pcall(L, 1, 0, 0);

		if (error) {
			printf("%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
	}
}

int main(int argc, char *argv[])
{
	skt_register(argv [1]);
	zmq_srv_init("127.0.0.1", 5556);
#ifdef SELECT_MULTITHREAD
	zmq_threadstart(work_task, 1);
#else
	zmq_process_start(work_task, (void *)1);
#endif

	zmq_srv_start();

	// sleep(100);
	zmq_srv_exit();
}

