#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include <assert.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "set_expire_time.h"

void set_expire_time(char *key, int len, int redis_num)
{
	lua_State *L;

	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_dofile(L, "./timport_set_expire_time.lua");
	lua_getglobal(L, "set_time");
	lua_newtable(L);

	lua_pushnumber(L, 1);
	lua_pushlstring(L, key, len);
	lua_settable(L, -3);

	lua_pushnumber(L, 2);
	lua_pushnumber(L, redis_num);
	lua_settable(L, -3);

	int iError = lua_pcall(L, 1, 0, 0);

	if (iError) {
		printf("%s\n", lua_tostring(L, -1));
		lua_close(L);
		exit(0);
	}

	lua_pop(L, 1);
	lua_close(L);

	return 0;
}

