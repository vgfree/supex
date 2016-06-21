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

#include "common.h"
#include "get_start_time.h"

int get_start_time(int timestamp)
{
	lua_State *L;
	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_dofile(L, "./timport_gettime.lua");
	lua_getglobal(L, "get_time");

        lua_pushnumber(L, timestamp);

	int iError = lua_pcall(L, 1, 1, 0);
	if (iError) {
		printf("%d\n", lua_tonumber(L, -1));
		lua_close(L);
		exit(0);
	}

	lua_stack(L);

	int start_time = lua_tonumber(L, -1);
	lua_pop(L, 1);
	lua_close(L);
	
	return start_time;
}
/*
int main()
{
	printf("start time = %d\n", get_start_time(1466166100));
}
*/
