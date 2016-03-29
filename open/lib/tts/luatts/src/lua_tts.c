/**
 * copyright (c) 2015
 * All Right Reserved
 *
 * @file lua_tts.c
 * @detail lua package of C
 *
 * @date   2015-12-24
 */

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tts_sdk.h"

struct tts_handle
{
	/* data */
	TTSHANDLE       tts;
	char            *data;
	size_t          len;
};

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

static int tts_create(lua_State *L)
{
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "%s", "argument error");
	}

	struct tts_handle *handle;
	handle = (struct tts_handle *)malloc(sizeof(*handle));

	if (!handle) {
		return luaL_error(L, "%s", "no enough memory");
	}

	handle->data = NULL;
	handle->len = 0;

	int ret = usc_tts_create_service(&handle->tts);

	if (ret != USC_SUCCESS) {
		free(handle);
		return luaL_error(L, "%s", "usc_tts_create_service");
	}

	ret = usc_tts_set_option(handle->tts, "appkey", lua_tostring(L, 1));
	ret = usc_tts_set_option(handle->tts, "secret", lua_tostring(L, 2));

	ret = usc_tts_start_synthesis(handle->tts);

	if (ret != USC_SUCCESS) {
		usc_tts_release_service(handle->tts);
		free(handle);
		return luaL_error(L, "%s", "usc_tts_start_synthesis");
	}

	lua_pushlightuserdata(L, handle);

	return 1;
}

static int tts_text_put(lua_State *L)
{
	if (lua_gettop(L) != 2) {
		return luaL_error(L, "%s", "argument error");
	}

	struct tts_handle *handle = (struct tts_handle *)lua_touserdata(L, 1);

	if (handle == NULL) {
		return luaL_error(L, "%s", "argument invalid, excepted pointer.");
	}

	size_t          txtlen = 0;
	const char      *txt = lua_tolstring(L, 2, &txtlen);

	int ret = usc_tts_text_put(handle->tts, txt, txtlen);

	if (ret != USC_SUCCESS) {
		usc_tts_stop_synthesis(handle->tts);
		lua_pushboolean(L, 0);
	} else {
		lua_pushboolean(L, 1);
	}

	return 1;
}

static int tts_get_result(lua_State *L)
{
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "%s", "argument error");
	}

	struct tts_handle *handle = (struct tts_handle *)lua_touserdata(L, 1);

	if (handle == NULL) {
		return luaL_error(L, "%s", "argument invalid, excepted pointer.");
	}

	unsigned int    audioLen = 0;
	int             synth_status;
	int             ret;

	const void *data = usc_tts_get_result(handle->tts, &audioLen, &synth_status, &ret);

	if (data != NULL) {
		char *tmp = handle->data;
		handle->data = (char *)realloc(tmp, audioLen);

		if (handle->data == NULL) {
			handle->data = tmp;
			return luaL_error(L, "%s", "no enough memory.");
		} else {
			handle->len = audioLen;
			memcpy(handle->data, data, audioLen);
		}

		lua_pushlightuserdata(L, handle->data);
		lua_pushinteger(L, handle->len);
	} else {
		lua_pushlightuserdata(L, NULL);
		lua_pushinteger(L, 0);
	}

	if ((synth_status == AUDIO_DATA_RECV_DONE) || (ret != 0)) {
		lua_pushboolean(L, 0);
	} else {
		lua_pushboolean(L, 1);
	}

	return 3;
}

static int tts_destroy(lua_State *L)
{
	if (lua_gettop(L) != 1) {
		return luaL_error(L, "%s", "argument error");
	}

	struct tts_handle *handle = (struct tts_handle *)lua_touserdata(L, 1);

	if (handle == NULL) {
		return luaL_error(L, "%s", "argument invalid, excepted pointer.");
	}

	int ret = usc_tts_stop_synthesis(handle->tts);

	if (ret != USC_SUCCESS) {
		return luaL_error(L, "%s", "usc_tts_stop_synthesis");
	}

	usc_tts_release_service(handle->tts);
	free(handle->data);
	free(handle);

	return 1;
}

static const struct luaL_Reg lib[] = {
	{ "tts_create",     tts_create     },
	{ "tts_text_put",   tts_text_put   },
	{ "tts_get_result", tts_get_result },
	{ "tts_destroy",    tts_destroy    },
	{ NULL,             NULL           }
};

int luaopen_luatts(lua_State *L)
{
	lua_newtable(L);
	luaL_setfuncs(L, lib, 0);

	return 1;
}

