#ifndef __lua_kv_utils__
#define __lua_kv_utils__


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libkv.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "common.h"


__BEGIN_DECLS

int luakv_createbyptr(lua_State *L);
int luakv_create(lua_State *L);
int luakv_ask(lua_State *L);
int luakv_getiter(lua_State *L);
int luakv_iternext(lua_State *L);

__END_DECLS


#endif
