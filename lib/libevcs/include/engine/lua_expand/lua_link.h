#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int async_http(lua_State *L);

int sync_http(lua_State *L);
