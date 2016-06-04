#include <assert.h>

#include "minor/sniff_api.h"
#include "spx_evcs.h"
#include "lua_expand/lj_c_coro.h"
#include "lua_expand/lua_link.h"
#include "luakv/luakv.h"
#include "sniff_evcoro_lua_api.h"
#ifdef OPEN_TOPO
  #include "topo.h"
  #include "topo_api.h"
#endif


#ifdef OPEN_TOPO
static int lua_get_export_road_by_road(lua_State *L)
{
	int                     ok = false;
	uint64_t                slot[MAX_ONE_NODE_OWN_LINE_COUNT];
	struct query_args       info = {};

	info.idx = luaL_checkinteger(L, 1);
	info.buf = slot;
	info.peak = MAX_ONE_NODE_OWN_LINE_COUNT;

	ok = get_export_road_by_road(&info);

	if (false == ok) {
		lua_pushboolean(L, 0);
		lua_pushnil(L);
		return 2;
	}

	lua_pushboolean(L, 1);
	lua_newtable(L);
	int i = 0;

	for (; i < info.size; i++) {
		lua_pushnumber(L, i + 1);
		lua_pushinteger(L, info.buf[i]);
		lua_settable(L, -3);
	}

	return 2;
}
#endif	/* ifdef OPEN_TOPO */

static lua_State *_vms_new(void)
{
	int             error = 0;
	lua_State       *L = luaL_newstate();

	assert(L);

	/*设置分配函数*/
	// lua_setallocf( L, lua_alloc, NULL);

	luaopen_base(L);
	luaL_openlibs(L);
	/*reg func*/
	lua_register(L, "supex_http", async_http);
	lua_register(L, "lua_default_switch", lj_evcoro_switch);
#ifdef OPEN_TOPO
	lua_register(L, "get_export_road_by_road", lua_get_export_road_by_road);
#endif
	lua_register(L, "search_kvhandle", search_kvhandle);
	/*lua init*/
	{
		extern struct sniff_cfg_list g_sniff_cfg_list;

		int app_lua_get_serv_name(lua_State *L)
		{
			lua_pushstring(L, g_sniff_cfg_list.argv_info.serv_name);
			return 1;
		}

		lua_register(L, "app_lua_get_serv_name", app_lua_get_serv_name);

		error = luaL_dofile(L, "sniff_lua/core/init.lua");

		if (error) {
			x_printf(E, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
			exit(EXIT_FAILURE);
		}
	}
	/*app init*/
	error = luaL_dofile(L, "sniff_lua/core/start.lua");

	if (error) {
		x_printf(E, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		exit(EXIT_FAILURE);
	}

	return L;
}


int sniff_vms_init(void *user, union virtual_system **VMS, struct sniff_task_node *task)
{
	int             error = 0;
	lua_State       **L = VMS;
	if (*L != NULL) {
		x_printf(S, "No need to init LUA VM!\n");
		return 0;
	}

	*L = _vms_new();
	assert(*L);
	lua_getglobal(*L, "app_evcoro_init");
	lua_pushinteger(*L, supex_get_default()->scheduler);
	lua_pushinteger(*L, 0);
	error = lua_pcall(*L, 2, 0, 0);
	if (error) {
		x_printf(E, "%s", lua_tostring(*L, -1));
		lua_pop(*L, 1);
		exit(EXIT_FAILURE);
	}
	return error;
}

