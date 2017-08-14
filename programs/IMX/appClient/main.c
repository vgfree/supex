/*
 * CopyRight    : DT+
 * Author       : huiqi.qian
 * Date         : 07-15-2017
 * Description  : Appclient
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

#define SERVER_NAME "AK47"

static lua_State *lua_vm_init(void)
{
	int             error = 0;
	lua_State       *L = luaL_newstate();

	assert(L);
	luaopen_base(L);
	luaL_openlibs(L);

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

	while (1) {
		zmq_srv_fetch(&devc);

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
	work_task();
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
