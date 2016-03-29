#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "map_seg.h"

int iterator_init(lua_State *L);

int iterator_next(lua_State *L);

int iterator_destory(lua_State *L);

int get_SGInfo(lua_State *L);

