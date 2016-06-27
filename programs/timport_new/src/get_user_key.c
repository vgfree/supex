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

void get_user_key(int timestamp, char *user_key, int max_key_len)
{
	lua_State *L;

	L = luaL_newstate();
	luaL_openlibs(L);
	luaL_dofile(L, "./timport_getkey.lua");
	lua_getglobal(L, "get_key");

	lua_pushnumber(L, timestamp);

	int iError = lua_pcall(L, 1, 1, 0);

	if (iError) {
		printf("%s\n", lua_tostring(L, -1));
		lua_close(L);
		exit(0);
	}

	lua_stack(L);

	int len = strlen(lua_tostring(L, -1));

	if (max_key_len < len) {
		printf("Key length is too long!\n");
		return;
	}

	memcpy(user_key, (char *)lua_tostring(L, -1), len);

	lua_pop(L, 1);
	lua_close(L);
}

/*
 * int main() {
 *	char * key = NULL;
 *	int len = 0;
 *	int timestamp = 1466161800;
 *	int interval = 10;
 *	get_user_key(timestamp, interval);
 *	printf("key = %s, len = %d\n", g_user_key.key, g_user_key.len);
 *	return 0;
 *   }
 */

