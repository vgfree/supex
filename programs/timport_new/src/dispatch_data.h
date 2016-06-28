#pragma once

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

lua_State *lua_vm_init(void);
int dispatch_data(lua_State *L, char *user, int user_len, char *time, int time_len, int redis_num);

