#ifdef __cplusplus
extern "C" {
  #include <lua.h>
  #include <lauxlib.h>
  #include <lualib.h>
}
#else

  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "../../convert.h"

// using namespace std;
/* ===== INITIALISATION ===== */

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502

/* Compatibility for Lua 5.1.
 *
 * luaL_setfuncs() is used to create a module table where the functions have
 * json_config_t as their first upvalue. Code borrowed from Lua 5.2 source.
 */
static void luaL_setfuncs(lua_State *l, const luaL_Reg *reg, int nup)
{
	int i;

	luaL_checkstack(l, nup, "too many upvalues");

	for (; reg->name != NULL; reg++) {		/* fill the table with given functions */
		for (i = 0; i < nup; i++) {		/* copy upvalues to the top */
			lua_pushvalue(l, -nup);
		}

		lua_pushcclosure(l, reg->func, nup);		/* closure with those upvalues , and pop all of them*/
		lua_setfield(l, -(nup + 2), reg->name);
	}

	lua_pop(l, nup);	/* remove upvalues */
}
#endif

static int lua_png2flif(lua_State *L)
{
	if (lua_gettop(L) != 4) {
		return luaL_error(L, "%s", "argument error");
	}

	char *buffer = (char *)lua_tostring(L, 1);

	if (buffer == NULL) {
		return luaL_error(L, "%s", "buffer == NULL");
	}

	size_t lSize = (size_t)lua_tonumber(L, 2);

	if (lSize == 0) {
		return luaL_error(L, "%s", "lSize == 0");
	}

	uint32_t w = (uint32_t)lua_tonumber(L, 3);

	if (w == 0) {
		return luaL_error(L, "%s", "w == 0");
	}

	uint32_t h = (uint32_t)lua_tonumber(L, 4);

	if (h == 0) {
		return luaL_error(L, "%s", "h == 0");
	}

	png2flif(buffer, lSize, w, h);

	return 0;
}

static int lua_flif2png(lua_State *L)
{
	if (lua_gettop(L) != 4) {
		return luaL_error(L, "%s", "argument error");
	}

	char *buffer = (char *)lua_tostring(L, 1);

	if (buffer == NULL) {
		return luaL_error(L, "%s", "buffer == NULL");
	}

	size_t lSize = (size_t)lua_tonumber(L, 2);

	if (lSize == 0) {
		return luaL_error(L, "%s", "lSize == 0");
	}

	uint32_t w = (uint32_t)lua_tonumber(L, 3);

	if (w == 0) {
		return luaL_error(L, "%s", "w == 0");
	}

	uint32_t h = (uint32_t)lua_tonumber(L, 4);

	if (h == 0) {
		return luaL_error(L, "%s", "h == 0");
	}

	flif2png(buffer, lSize, w, h);

	return 0;
}

/*
 *   static int lua_flif2flif(lua_State *L)
 *   {
 *        if (lua_gettop(L) != 2)
 *        {
 *                return luaL_error(L, "%s", "argument error");
 *        }
 *
 *        char *input = (char *)lua_tostring(L, 1);
 *        if (input == NULL)
 *        {
 *                return luaL_error(L, "%s", "input == NULL");
 *        }
 *
 *        char *output = (char *)lua_tostring(L, 2);
 *        if (output == NULL)
 *        {
 *                return luaL_error(L, "%s", "output == NULL");
 *        }
 *
 *        flif2flif(input, output);
 *
 *        return 0;
 *   }
 */

static const struct luaL_Reg lib[] = {
	{ "lua_png2flif", lua_png2flif },
	{ "lua_flif2png", lua_flif2png },
	//        { "lua_flif2flif", lua_flif2flif},
	{ NULL,           NULL         }
};

int luaopen_luaflif(lua_State *L)
{
	lua_newtable(L);
	luaL_setfuncs(L, lib, 0);

	return 1;
}

