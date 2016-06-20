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

#include "dispatch_data.h"

int dispatch_data(char *user, int user_len, char *time, int time_len, int redis_num)
{
	//printf("user = %s\n", user);
	lua_State *L;

	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_dofile(L, "./timport_server.lua");
	lua_getglobal(L, "GetTable");
	lua_newtable(L);

	lua_pushnumber(L, 1);
	lua_pushlstring(L, user, user_len);
	lua_settable(L, -3);

	lua_pushnumber(L, 2);
	lua_pushlstring(L, time, time_len);
	lua_settable(L, -3);
	
	lua_pushnumber(L, 3);
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
}

/*
*    int main() {
*         char * name = "508542171690587";
*         char * time = "ACTIVEUSER:20160617193";
*         dispatch_data(name, strlen(name), time, strlen(time));
*         return 0;
*    }
* 
*/
