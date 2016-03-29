#include "luakvutils.h"

#define LUA_KVHDL_METATABLE	("LUA_LIBKV_HDL_METATABLE")
#define LUA_KVASK_METATABLE     ("LUA_LIBKV_ANSWER_HANDLE")
#define LUA_KVITER_METATABLE    ("LUA_LIBKV_ITERATOR_HANDLE")

typedef struct
{
	/* data */
	union
	{
		kv_handler_t		*hdl;
		kv_answer_t		*ans;
		kv_answer_iter_t	*iter;
	};
} *HANDLE;

static
int luakv_releasehdl(lua_State *L);

static
int luakv_releaseask(lua_State *L);

static
int luakv_releaseiter(lua_State *L);

/**
 * 通过给出的libkv句柄初始化一个luakv句柄，并返回luakv句柄
 * 如果有错误，则抛出lua异常
 */
int luakv_createbyptr(lua_State *L)
{
	kv_handler_t	*hdl = NULL;
	HANDLE 		ptr = NULL;
	int             argc = lua_gettop(L);

        if (argc != 1) {
                luaL_error(L, "Bad argument, 'value' expected");
        }

	hdl = lua_touserdata(L, 1);

	if (hdl == NULL) {
                luaL_error(L, "Bad argument, 'userdata' expected(a pointer of luakv handler.)");
        }

	x_printf(D, "Initialize libkv\n");

	ptr = (HANDLE)lua_newuserdata(L, sizeof(*ptr));
	ptr->hdl = hdl;

	luaL_newmetatable(L, LUA_KVHDL_METATABLE);

	lua_pushcclosure(L, luakv_releasehdl, 0);
	lua_setfield(L, -2, "__gc");

	lua_setmetatable(L, -2);

	return 1;
}
/**
 * 初始化一个luakv句柄，并返回luakv句柄
 * 如果有错误，则抛出lua异常
 */
int luakv_create(lua_State *L)
{
	kv_handler_t	*hdl = NULL;
	// HANDLE 		ptr = NULL;

	if ((hdl = kv_create(NULL)) == NULL)
	{
		return luaL_error(L, "libaray key-value initialize failed");
	}

	// x_printf(D, "Initialize libkv\n");

	// ptr = (HANDLE)lua_newuserdata(L, sizeof(*ptr));
	// ptr->hdl = hdl;

	// luaL_newmetatable(L, LUA_KVHDL_METATABLE);

	// lua_pushcclosure(L, luakv_releasehdl, 0);
	// lua_setfield(L, -2, "__gc");

	// lua_setmetatable(L, -2);
	
	lua_settop(L, 0);

	lua_pushlightuserdata(L, hdl);

	return luakv_createbyptr(L);
}

static
int luakv_releasehdl(lua_State *L)
{
	kv_handler_t	*hdl = NULL;
	HANDLE 		ptr = NULL;

	ptr = (HANDLE)luaL_checkudata(L, 1, LUA_KVHDL_METATABLE);

	hdl = ptr->hdl;

	if (! hdl)
	{
		x_printf(E, "Invalid libkv handler.\n");

		return luaL_error(L, "Invalid libkv handler.");
	}

	x_printf(D, "Release libkv.\n");

	kv_destroy(hdl);

	return 0;
}
/*
 * 执行命令，并在栈顶返回ask操作句柄，并从栈中移除handle
 * 如果命令结果为空，则返回nil
 * 如果命令有错误，则抛出lua异常
 *
 */
int luakv_ask(lua_State *L)
{
	int         argc = 0;
	kv_answer_t	*ans = NULL;
	kv_handler_t	*hdl = NULL;
	HANDLE      ptr = NULL;
	size_t      len = 0;
	const char  *command = NULL;

	argc = lua_gettop(L);
	if (argc < 2)
	{
		luaL_error(L, "Bad argument, less than 2 arguments.");
	}
	

	ptr = (HANDLE)luaL_checkudata(L, -2, LUA_KVHDL_METATABLE);
	hdl = ptr->hdl;

	if (!hdl)
	{
		x_printf(E, "Invalid libkv handler.\n");

		return luaL_error(L, "Invalid libkv handler.");
	}

#if 0
	luaL_Buffer buffer;
	int         top = 0;
	int         i = 0;

	//组合所有的参数
	top = lua_gettop(L);
	luaL_buffinit(L, &buffer);
	for (i = 0; i < top; ++i)
	{
	    /* code */
	    luaL_addstring(&buffer, luaL_checkstring(L, i + 1));
	    luaL_addchar(&buffer, ' ');
	}
	luaL_pushresult(&buffer);
#endif

	//获取栈顶所有参数组合的命令
	command = luaL_checklstring(L, -1, &len);

	//弹出使用过的句柄
	lua_remove(L, -2);

	ans = kv_ask(hdl, command, (unsigned)len);
	if (ERR_NONE != ans->errnum && ERR_NIL != ans->errnum)
	{
		lua_pushfstring(L, "Run [%s] command failed : %s.",
				command,
				SWITCH_NULL_STR(ans->err));

		x_printf(E, "Run [%s] command failed : %s.\n",
				command,
				SWITCH_NULL_STR(ans->err));

		kv_answer_release(ans);

		return lua_error(L);
	}
	
	if (ERR_NIL == ans->errnum)
	{
		/* code */
		kv_answer_release(ans);
		lua_pushnil(L);
	}
	else
	{
		x_printf(D, "Get answer, Run [%s] command.\n", command);
		/* 推入一个可安全操作的userdate*/
		ptr = (HANDLE)lua_newuserdata(L, sizeof(*ptr));
		ptr->ans = ans;

		//{ __gc = luakv_releaseask }
		luaL_newmetatable(L, LUA_KVASK_METATABLE);

		lua_pushcclosure(L, luakv_releaseask, 0);
		lua_setfield(L, -2, "__gc");

		lua_pushstring(L, "v");
		lua_setfield(L, -2, "__mode");

		lua_pushvalue(L, 2);
		lua_setfield(L, -2, "command");

		lua_setmetatable(L, -2);
	}

	return 1;
}

static
int luakv_releaseask(lua_State *L)
{
	kv_answer_t    *ans = NULL;
	HANDLE      ptr = NULL;

	ptr = (HANDLE)luaL_checkudata(L, 1, LUA_KVASK_METATABLE);
	ans = ptr->ans;

	if (! ans)
	{
		x_printf(E, "Invalid libkv ask-handler.\n");

		return luaL_error(L, "Invalid libkv ask-handler.");
	}

	x_printf(D, "Release answer\n");

	kv_answer_release(ans);
	return 0;
}

/*
 * 用结果句柄获取适配器句柄
 */
int luakv_getiter(lua_State *L)
{
	kv_answer_t		*ans = NULL;
	kv_answer_iter_t	*iter = NULL;
	HANDLE          ptr = NULL;

	ptr = (HANDLE)luaL_checkudata(L, -1, LUA_KVASK_METATABLE);
	ans = ptr->ans;

	if (! ans)
	{
		x_printf(E, "Invalid libkv handler.\n");

		return luaL_error(L, "Invalid libkv handler.");
	}

	//弹出结果句柄
	// lua_pop(L, 1);

	iter = kv_answer_get_iter(ans, ANSWER_HEAD);
	if (! iter)
	{
		x_printf(E, "Can't create a iterator by answer-handler %p.\n", ans);

		return luaL_error(L, "Can't create a iterator by answer-handler %p.", ans);
	}

	x_printf(D, "Get iterator of answer.\n");

	ptr = (HANDLE)lua_newuserdata(L, sizeof(*ptr));
	ptr->iter = iter;

	// 设置元表
	luaL_newmetatable(L, LUA_KVITER_METATABLE);

	lua_pushcclosure(L, luakv_releaseiter, 0);
	lua_setfield(L, -2, "__gc");

	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");

	// lua_pushvalue(L, -2);
	lua_pushvalue(L, -3);
	lua_setfield(L, -2, "answer");

	lua_setmetatable(L, -2);

	//弹出结果句柄
	lua_remove(L, -2);

	return 1;
}

static
int luakv_releaseiter(lua_State *L)
{
	kv_answer_iter_t   *iter = NULL;
	HANDLE		ptr = NULL;

	ptr = (HANDLE)luaL_checkudata(L, 1, LUA_KVITER_METATABLE);
	iter = ptr->iter;

	if (! iter)
	{
		x_printf(E, "Invalid libkv iter-handler.\n");

		return luaL_error(L, "Invalid libkv iter-handler.");
	}

	kv_answer_release_iter(iter);

	x_printf(D, "Release iterator of answer.\n");

	return 0;
}

// static
int luakv_iternext(lua_State *L)
{
	kv_answer_iter_t   *iter = NULL;
	kv_answer_value_t  *value = NULL;
	const char      *result = NULL;
	HANDLE          ptr = NULL;

	ptr = (HANDLE)luaL_checkudata(L, -1, LUA_KVITER_METATABLE);
	iter = ptr->iter;

	if (! iter)
	{
		x_printf(E, "Invalid libkv iterator-handler\n");

		return luaL_error(L, "Invalid libkv iterator-handler.");
	}

	value = kv_answer_next(iter);
	if (! value)
	{
		lua_pop(L, 1);
		lua_pushnil(L);
		// x_printf(D, "The iterator %p is over.\n", iter);
		return 1;
	}

	if (! (result = kv_answer_value_to_string(value)))
	{
		lua_pop(L, 1);
		lua_pushnil(L);
		x_printf(E, "Can't get value from iterator-handler %p\n", value);
		return 1;
	}

	// x_printf(D, "Get the next value of answer by iterator : %s\n", result);

	lua_pushstring(L, result);

	return 1;
}

