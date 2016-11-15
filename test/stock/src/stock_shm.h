#pragma once

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "stock.h"

#define STOCK_KEY (5000)

int lua_stock_shm_init(lua_State *L);

int lua_stock_shm_push(lua_State *L);

bool stock_shm_open(void);

size_t stock_shm_get_count(void);

size_t stock_shm_get_limit(void);

stock_trade_t *stock_shm_pull(size_t idx);

