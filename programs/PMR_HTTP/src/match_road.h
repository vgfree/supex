#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <map_pmr.h>

int check_parameter(double lon, double lat, short dir);

int entry_cmd_locate(lua_State *L);

