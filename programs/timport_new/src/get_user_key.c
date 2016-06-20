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
#include "get_user_key.h"

struct user_key g_user_key;

void get_user_key(int timestamp, int interval)
{
	if (time <= 0 || interval < 0) {
		printf("The time is invalid.\n");
		return;
	} 

	lua_State *L;
	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_dofile(L, "./timport_utils.lua");
	lua_getglobal(L, "GetKey");
	lua_newtable(L);

	lua_pushnumber(L, 1);
        lua_pushnumber(L, timestamp);
        lua_settable(L, -3);

        lua_pushnumber(L, 2);
        lua_pushnumber(L, interval);
        lua_settable(L, -3);	

	int iError = lua_pcall(L, 1, 1, 0);
	if (iError) {
		printf("%s\n", lua_tostring(L, -1));
		lua_close(L);
		exit(0);
	}

	printLuaStack(L);
	
	memset(&g_user_key, 0, 100);
	g_user_key.len = strlen(lua_tostring(L, -1));
	memcpy(g_user_key.key, (char*)lua_tostring(L, -1), g_user_key.len);

	lua_pop(L, 1);
	lua_close(L);
}

/*
 *int main() {
 *	char * key = NULL;
 *	int len = 0; 
 *	int timestamp = 1466161800;
 *	int interval = 10;
 *	get_user_key(timestamp, interval);
 *	printf("key = %s, len = %d\n", g_user_key.key, g_user_key.len);
 *	return 0;
 }
*/
