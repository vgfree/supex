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

struct user_key UserKey;

void getUserKey(int timestamp, int timeInterval)
{
	if (time <= 0 || timeInterval < 0) {
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
        lua_pushnumber(L, timeInterval);
        lua_settable(L, -3);	

	int iError = lua_pcall(L, 1, 1, 0);
	if (iError) {
		printf("%s\n", lua_tostring(L, -1));
		lua_close(L);
		exit(0);
	}

	printLuaStack(L);
	
	memset(&UserKey, 0, 100);
	UserKey.keyLen = strlen(lua_tostring(L, -1));
	memcpy(UserKey.key, (char*)lua_tostring(L, -1), UserKey.keyLen);

	lua_pop(L, 1);
	lua_close(L);
}

/*
    int main() {
    	char * key = NULL;
	int keyLen = 0; 
    	int timestamp = 1466161800;
	int timeInterval = 10;
    	getUserKey(timestamp, timeInterval);
	printf("key = %s, len = %d\n", UserKey.key, UserKey.keyLen);
    	return 0;
    }
*/
