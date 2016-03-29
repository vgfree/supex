#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "mfptp_pack.h"
#define max 10240
static int data_head(lua_State *L)
{
	char *dst = NULL;

	dst = (char *)malloc(sizeof(char) * 10240);

	if ((3 != lua_gettop(L)) || (NULL == dst)) {
		lua_pushnil(L);
		return 1;
	}

	int     ver = luaL_checkint(L, 1);
	int     sk_type = luaL_checkint(L, 2);
	int     pkt_cnt = luaL_checkint(L, 3);

	int     headLength = mfptp_pack_hdr(dst, ver, sk_type, pkt_cnt);
	int     i = 0;

	for (i = 0; i < headLength; i++) {
		if (i < 6) {
			printf("%c ", dst[i]);
		} else {
			printf("%d ", dst[i]);
		}
	}

	printf("\n");

	if (headLength != 10) {
		lua_pushstring(L, "headLength is error");
		return 1;
	}

	lua_pushnumber(L, headLength);
	lua_pushlightuserdata(L, dst);
	return 2;
}

static int data_body(lua_State *L)
{
	char *dst1 = NULL;

	if (5 != lua_gettop(L)) {
		lua_pushnil(L);
		return 1;
	}

	int     bodyLength = 0;
	char    *src = (char *)luaL_checkstring(L, 1);
	int     len = luaL_checkint(L, 2);
	int     more = luaL_checkint(L, 3);
	int     length = luaL_checkint(L, 4);
	char    *dst = lua_touserdata(L, 5);

	dst1 = dst + length + 10;
	printf("pianyi:%d\n", (length + 10));
	bodyLength = mfptp_pack_frame(src, len, dst1, more);

	if (more > 0) {
		lua_pushnumber(L, bodyLength);
		lua_pushlstring(L, dst, max);
		return 2;
	}

	if (more == 0) {
		lua_pushlstring(L, dst, 10240);
		free(dst);
		return 1;
	}

	return 0;
}

static luaL_Reg lib[] = {
	{ "data_head", data_head },
	{ "data_body", data_body },
	{ NULL,        NULL      }
};

int luaopen_mfptppack(lua_State *L)
{
	const char *libName = "mfptppack";

	luaL_register(L, libName, lib);
	return 1;
}

