#include <assert.h>

#include "major/smart_api.h"
#include "lua_expand/lj_c_coro.h"
#include "lua_expand/lua_link.h"
#include "lua_expand/lj_http_info.h"
#include "lua_expand/lj_cache.h"
#include "luakv/luakv.h"
// #include "luakvcore.h"
#include "smart_evcoro_lua_api.h"

#ifdef OPEN_ITERATOR
  #include "lua_iterator.h"
#endif

static lua_State *_vms_new(void)
{
	int             error = 0;
	lua_State       *L = luaL_newstate();

	assert(L);
	luaopen_base(L);
	luaL_openlibs(L);
	/*reg func*/
	lua_register(L, "supex_http", async_http);
	lua_register(L, "lua_default_switch", lj_evcoro_switch);
	lua_register(L, "app_lua_get_head_data", app_lua_get_head_data);
	lua_register(L, "app_lua_get_body_data", app_lua_get_body_data);
	lua_register(L, "app_lua_get_path_data", app_lua_get_path_data);
	lua_register(L, "app_lua_get_uri_args", app_lua_get_uri_args);
	lua_register(L, "app_lua_get_recv_data", app_lua_get_recv_data);
	lua_register(L, "app_lua_add_send_data", app_lua_add_send_data);
	lua_register(L, "search_kvhandle", search_kvhandle);

#ifdef OPEN_ITERATOR
	lua_register(L, "iterator_init", iterator_init);
	lua_register(L, "iterator_next", iterator_next);
	lua_register(L, "iterator_destory", iterator_destory);
	lua_register(L, "get_SGInfo", get_SGInfo);
#endif

	/*lua init*/
	{
		extern struct smart_cfg_list g_smart_cfg_list;

		int app_lua_get_serv_name(lua_State *L)
		{
			lua_pushstring(L, g_smart_cfg_list.argv_info.serv_name);
			return 1;
		}

		lua_register(L, "app_lua_get_serv_name", app_lua_get_serv_name);

		error = luaL_dofile(L, "lua/core/init.lua");

		if (error) {
			x_printf(E, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
			exit(EXIT_FAILURE);
		}
	}
	/*app init*/
	error = luaL_dofile(L, "lua/core/start.lua");

	if (error) {
		x_printf(E, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		exit(EXIT_FAILURE);
	}

	return L;
}

int smart_vms_init(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	int             error = 0;
	lua_State       **L = VMS;

	if (*L != NULL) {
		x_printf(S, "No need to init LUA VM!\n");
		return 0;
	}

	printf("++++++++%d\n", task->index);
	*L = _vms_new();
	assert(*L);
	lua_getglobal(*L, "app_evcoro_init");
	lua_pushinteger(*L, supex_get_default()->scheduler);
	lua_pushinteger(*L, task->index);
	error = lua_pcall(*L, 2, 0, 0);

	if (error) {
		x_printf(E, "%s", lua_tostring(*L, -1));
		lua_pop(*L, 1);
		exit(EXIT_FAILURE);
	}

	return error;
}

