//
//  lualog_utils.h
//  bLib
//
//  Created by 周凯 on 15/8/25.
//  Copyright (c) 2015年 zk. All rights reserved.
//

#ifndef __bLib__lualog_utils__
#define __bLib__lualog_utils__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <sys/stat.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/*
 * C导出到C++
 */
#if defined(__cplusplus)
  #if !defined(__BEGIN_DECLS)
    #define __BEGIN_DECLS \
	extern "C" {
    #define __END_DECLS	\
	}
  #endif
#else
  #define __BEGIN_DECLS
  #define __END_DECLS
#endif

__BEGIN_DECLS

#undef  DIM
#define DIM(x) ((int)(sizeof((x)) / sizeof((x)[0])))

/**
 * 将L栈中的字符串格式和参数解析。
 * 如果失败则返回0，栈保存不变
 * 如果成功则返回1，并将结果整合到栈顶的luaL_Buffer中。
 */

int luaL_stringFormat(lua_State *L);

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
void luaL_setfuncs(lua_State *l, const luaL_Reg *reg, int nup);
#endif

/**
 * 为栈顶的表增加一个表成员
 * @param name 新增表的名称，如果为NULL则追加到栈顶表的尾部
 * @param metaname 新增表的元表名称，如果为NULL则不为其增加元表
 */
void luaL_tableAddTable(lua_State *L, const char *name, const char *metaname, bool isweak);

/**
 * 获取程序名称
 */
char *luaL_getProgName(lua_State *L, char *buff, size_t size);

__END_DECLS
#endif	/* defined(__bLib__lualog_utils__) */

