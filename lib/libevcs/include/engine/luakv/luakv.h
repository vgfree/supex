#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "../base/utils.h"
/* ------------                            */

/*
 * libkv句柄池
 */
struct kvpool_handle
{
	char    name[64];
	void    *hdl;
};

extern SListT g_kvhandle_pool;
typedef void *(*CREATE_HANDLE_FCB)(void);

/**
 * 初始化
 */
bool kvpool_init(CREATE_HANDLE_FCB fcb);

/**
 * 在lua层以句柄名称调用，查找或创建对应的句柄，并返回该句柄的libkv句柄的C指针
 */
int search_kvhandle(lua_State *L);

/**
 * 在C层已句柄名称调用，查找或创建对应的句柄，并返回该句柄
 */
struct kvpool_handle    *kvpool_search(SListT list, const char *name);

