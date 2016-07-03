#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "luakv/luakv.h"
#include "skt.h"
#include "load_cfg.h"
#include "supex.h"

#define SERVER_NAME "AK47"

static struct loghub_cfg_list g_loghub_cfg_list = {};

static lua_State *lua_vm_init(void)
{
	int     error = 0;
	bool    ok = false;

	ok = kvpool_init();

	if (!ok) {
		exit(EXIT_FAILURE);
	}

	lua_State *L = luaL_newstate();
	assert(L);
	luaopen_base(L);
	luaL_openlibs(L);
	/*reg func*/
	// lua_register(L, "app_lua_lru_cache_set_value", lrucache_setvalue);
	// lua_register(L, "app_lua_lru_cache_get_value", lrucache_getvalue);
	// lua_register(L, "app_lua_lru_cache_remove_value", lrucache_removevalue);
	lua_register(L, "search_kvhandle", search_kvhandle);
	/*lua init*/
	{
		int app_lua_get_serv_name(lua_State *L)
		{
			lua_pushstring(L, SERVER_NAME);
			return 1;
		}

		lua_register(L, "app_lua_get_serv_name", app_lua_get_serv_name);

		error = luaL_dofile(L, "lua/init.lua");

		if (error) {
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
			exit(EXIT_FAILURE);
		}
	}
	/*app init*/
	error = luaL_dofile(L, "lua/start.lua");

	if (error) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		exit(EXIT_FAILURE);
	}

	// lua_getglobal(L, "app_init");
	// lua_pcall(L, 0, 0, 0);
	return L;
}

void *work_task(void *args)
{
	int             error = 0;
	void            *data = NULL;
	lua_State       *L = lua_vm_init();

	// printf("%d \n", args);
	struct skt_device devc = {};

	while (1) {
		zmq_srv_fetch(&devc);

		lua_getglobal(L, "app_call");
		lua_newtable(L);

		int i = 0;

		// printf("cnt %d\n", devc.idx);
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
	load_loghub_cfg_argv(&g_loghub_cfg_list.argv_info, argc, argv);
	load_loghub_cfg_file(&g_loghub_cfg_list.file_info, g_loghub_cfg_list.argv_info.conf_name);

	skt_register(argv [1]);
	zmq_srv_init("0.0.0.0", g_loghub_cfg_list.file_info.port);

#ifdef SELECT_MULTITHREAD
	zmq_threadstart(work_task, 1);
	zmq_threadstart(work_task, 2);
	zmq_threadstart(work_task, 3);
	zmq_threadstart(work_task, 4);
	zmq_threadstart(work_task, 5);
	zmq_threadstart(work_task, 6);
	zmq_threadstart(work_task, 7);
	zmq_threadstart(work_task, 8);
#else
	zmq_process_start(work_task, (void *)1);
	zmq_process_start(work_task, (void *)2);
	zmq_process_start(work_task, (void *)3);
	zmq_process_start(work_task, (void *)4);
	zmq_process_start(work_task, (void *)5);
	zmq_process_start(work_task, (void *)6);
	zmq_process_start(work_task, (void *)7);
	zmq_process_start(work_task, (void *)8);
#endif

	zmq_srv_start();
	zmq_srv_exit();
}

