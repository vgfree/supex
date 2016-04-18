#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "skt.h"

#define SERVER_NAME "AK47"
#define HEAD_LENGTH 7

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

void __get_picture_head(void *head)
{
  int ret = memcmp("head", head, sizeof(HEAD_LENGTH));
  if (ret == 0) {
    printf("this data is a picture\n");		
  }
  else {
    printf("this data is not a picture\n");
  }
}

void *work_task(void *args)
{
	int             error = 0;
	void            *data = NULL;
	lua_State       *L = lua_vm_init();

	// printf("%d \n", args);
	struct skt_device devc = {};

	while (1) {
		printf("before zmq_srv_fetch\n");
		zmq_srv_fetch(&devc);
		printf("after zmq_srv_fetch\n");
		lua_getglobal(L, "app_call");
		printf("after lua_getglobal\n");
		lua_newtable(L);
		printf("after lua_newtable\n");
		__get_picture_head(devc.ibuffer);
		int i = 0;
		printf("cnt %d\n", devc.idx);
		
		for (i = 0; i < devc.idx; i++) {
			data = devc.ibuffer[i].iov_base;
                        printf("devc.ibuffer[i].iov_len = 0x%x\n", devc.ibuffer[i].iov_len);
			lua_pushnumber(L, i + 1);
			printf("after lua_pushnumber\n");
			lua_pushlstring(L, data, devc.ibuffer[i].iov_len);
			printf("after lua_pushlstring\n");
			lua_settable(L, -3);
			printf("after lua_settable\n");

			free(data);
		}
                printf("data = %s", data);
		error = lua_pcall(L, 1, 0, 0);

		if (error) {
			printf("lawrence hamster said error : %s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
		}
		else {
			printf("lua_pcall success\n");
		}
	}
}

int main(int argc, char *argv[])
{
	skt_register(argv [1]);
	//zmq_srv_init("127.0.0.1", 6992);
	zmq_srv_init("127.0.0.1", 5556);

#ifdef SELECT_MULTITHREAD
	zmq_threadstart(work_task, 1);
	zmq_threadstart(work_task, 2);
#else
	zmq_process_start(work_task, (void *)1);
	zmq_process_start(work_task, (void *)2);
#endif

	zmq_srv_start();

	// sleep(100);
	zmq_srv_exit();
}

