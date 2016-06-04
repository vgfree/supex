#pragma once

#include "lj_base.h"
#include "evmdl.h"

int app_lua_get_head_data(lua_State *L);

int app_lua_get_body_data(lua_State *L);

int app_lua_get_path_data(lua_State *L);

int app_lua_get_uri_args(lua_State *L);

