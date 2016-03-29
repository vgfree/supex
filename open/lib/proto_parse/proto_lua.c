#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "proto.h"

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

static int protoInit(lua_State *L)
{
	struct list *head = proto_init();
	if (head == NULL) {
		return luaL_error(L, "%s", "no enough memory");
	}

	lua_pushlightuserdata(L, head);

	return 1;
}

static int protoAppend(lua_State *L)
{
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "%s", "argument error");
	}

	struct list *head = (struct list *)lua_touserdata(L, 1);

	if (head == NULL) {
		return luaL_error(L, "%s", "no enough memory");
	}

	size_t len = 0;
	const char *str = lua_tolstring(L, 2, &len);

	proto_append(head, str, (int)len);

	return 1;
}

static int protoPack(lua_State *L)
{
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "%s", "argument error");
	}

	struct list *head = (struct list *)lua_touserdata(L, 1);

	if (head == NULL) {
		return luaL_error(L, "%s", "no enough memory");
	}

	char *proto_str = proto_pack(head);

	if (proto_str == NULL) {
		return luaL_error(L, "%s", "no enough memory");
	}

	lua_pushlightuserdata(L, proto_str);

	return 1;
}

static int protoParse(lua_State *L)
{
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "%s", "argument error");
	}

	struct list *head = (struct list *)lua_touserdata(L, 1);
	char            *p_str = lua_touserdata(L, 2);

	if (p_str == NULL) {
		return luaL_error(L, "%s", "no enough memory");
	}

	proto_parse(head, p_str);

	return 1;
}

static int protoGet(lua_State *L)
{
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "%s", "argument error");
	}

	struct list *head = (struct list *)lua_touserdata(L, 1);

	if (head == NULL) {
		return luaL_error(L, "%s", "no enough memory");
	}

	proto_get(head);

	return 1;
}

static int protoDestroy(lua_State *L)
{
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "%s", "argument error");
	}

	struct list *head = (struct list *)lua_touserdata(L, 1);

	proto_destroy(head);

	return 1;
}

static int protoFree(lua_State *L)
{
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "%s", "argument error");
	}

	char *str = lua_touserdata(L, 1);

	if (str == NULL) {
		return luaL_error(L, "%s", "no enough memory");
	}

	strfree(str);

	return 1;
}

static const struct luaL_Reg lib[] = {
	{ "protoInit",    protoInit    },
	{ "protoAppend",  protoAppend  },
	{ "protoPack",    protoPack    },
	{ "protoParse",   protoParse   },
	{ "protoGet",     protoGet     },
	{ "protoDestroy", protoDestroy },
	{ "protoFree",    protoFree    },
	{ NULL,           NULL         }
};

int luaopen_luaproto(lua_State *L)
{
	lua_newtable(L);
	luaL_setfuncs(L, lib, 0);

	return 1;
}

