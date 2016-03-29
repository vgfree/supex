#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "zk.h"

int zk_init(char *zk_host, const char *zk_rnode);

void zk_close(void);

int zk_get_read_dn(lua_State *L);

