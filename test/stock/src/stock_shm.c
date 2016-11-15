#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#include "stock_shm.h"

/* ===== INITIALISATION ===== */

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502

/* Compatibility for Lua 5.1.
 *
 * luaL_setfuncs() is used to create a module table where the functions have
 * json_config_t as their first upvalue. Code borrowed from Lua 5.2 source.
 */
static void luaL_setfuncs(lua_State *l, const luaL_Reg *reg, int nup)
{
	int i;

	luaL_checkstack(l, nup, "too many upvalues");

	for (; reg->name != NULL; reg++) {		/* fill the table with given functions */
		for (i = 0; i < nup; i++) {		/* copy upvalues to the top */
			lua_pushvalue(l, -nup);
		}

		lua_pushcclosure(l, reg->func, nup);		/* closure with those upvalues , and pop all of them*/
		lua_setfield(l, -(nup + 2), reg->name);
	}

	lua_pop(l, nup);	/* remove upvalues */
}
#endif

static size_t *volatile g_stock_limit = NULL;
static size_t *volatile g_stock_count = NULL;
static stock_trade_t    *g_stock_slots = NULL;

/************************************************************************************************
 * SHARE_MEMORY_DATA_FORMAT: size_t limit | size_t count | stock_trade_t | stock_trade_t | ...	*
 ***********************************************************************************************/
int lua_stock_shm_init(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc != 1) {
		return luaL_error(L, "Bad argument, input max stock counts.");
	}

	int shmid = shmget(STOCK_KEY, 0, 0666);
	printf("shmid is %d\n", shmid);

	if (shmid >= 0) {
		shmctl(shmid, IPC_RMID, 0);
	}

	size_t  max = lua_tointeger(L, 1);
	size_t  size = sizeof(stock_trade_t) * max + sizeof(size_t) + sizeof(size_t);

	shmid = shmget(STOCK_KEY, size, IPC_CREAT | 0666);
	printf("shmid is %d\n", shmid);

	if (shmid < 0) {
		return luaL_error(L, "create shm failed!");
	} else {
		void *addr = shmat(shmid, NULL, 0);

		if (addr == (void *)-1) {
			fprintf(stderr, "shmat failed\n");
			addr = NULL;
		}

		printf("shm addr is--------------%p\n", addr);

		if (addr) {
			g_stock_limit = (size_t *)addr;
			*g_stock_limit = max;
			g_stock_count = (size_t *)((char *)addr + sizeof(size_t));
			*g_stock_count = 0;
			g_stock_slots = (stock_trade_t *)((char *)addr + sizeof(size_t) + sizeof(size_t));

			lua_pushboolean(L, 1);
			return 1;
		} else {
			lua_pushboolean(L, 0);
			return 1;
		}
	}
}

int lua_stock_shm_push(lua_State *L)
{
	int argc = lua_gettop(L);

	if (argc != 7) {
		return luaL_error(L, "Bad argument, less than 7 arguments.");
	}

	stock_trade_t st = {};
	st.mode = lua_toboolean(L, 1) ? STOCK_BUY_IN : STOCK_SELL_OUT;
	st.timestamp = lua_tointeger(L, 2);
	st.yesterday_last_pice = lua_tonumber(L, 3);
	st.opening_pice = lua_tonumber(L, 4);
	st.max_pice = lua_tonumber(L, 5);
	st.min_pice = lua_tonumber(L, 6);
	st.new_pice = lua_tonumber(L, 7);

	assert(*g_stock_limit > *g_stock_count);

	size_t cuser = *g_stock_count;
	memcpy(&g_stock_slots[cuser], &st, sizeof(stock_trade_t));
	*g_stock_count = cuser + 1;

	lua_pushboolean(L, 1);
	return 1;
}

static
const luaL_Reg regLib[] = {
	{ "shm_init", lua_stock_shm_init        },
	{ "shm_push", lua_stock_shm_push        },
	{ NULL,       NULL                      }
};

int luaopen_stock(lua_State *L)
{
	lua_newtable(L);
	luaL_setfuncs(L, regLib, 0);

	return 1;
}

/*
 * C API
 */
bool stock_shm_open(void)
{
	int shmid = shmget(STOCK_KEY, 0, 0666);

	printf("shmid is %d\n", shmid);

	if (shmid < 0) {
		perror("open shm failed!\n");
		return false;
	} else {
		void *addr = shmat(shmid, NULL, 0);

		if (addr == (void *)-1) {
			fprintf(stderr, "shmat failed\n");
			addr = NULL;
		}

		printf("shm addr is--------------%p\n", addr);

		if (addr == NULL) {
			return false;
		} else {
			g_stock_limit = (size_t *)addr;
			g_stock_count = (size_t *)((char *)addr + sizeof(size_t));
			g_stock_slots = (stock_trade_t *)((char *)addr + sizeof(size_t) + sizeof(size_t));

			return true;
		}
	}
}

size_t stock_shm_get_count(void)
{
	assert(g_stock_count);

	return *g_stock_count;
}

size_t stock_shm_get_limit(void)
{
	assert(g_stock_limit);

	return *g_stock_limit;
}

stock_trade_t *stock_shm_pull(size_t idx)
{
	assert(g_stock_slots);

	size_t max = stock_shm_get_limit();
	assert((max >= idx) && (idx > 0));

	return &g_stock_slots[idx - 1];
}

