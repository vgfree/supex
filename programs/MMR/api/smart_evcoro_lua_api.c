#include <assert.h>

#include "match.h"
#include "major/smart_api.h"
#include "lua_expand/lj_c_coro.h"
#include "lua_expand/lua_link.h"
#include "lua_expand/lj_http_info.h"
#include "lua_expand/lj_cache.h"
// #include "luakvcore.h"
#include "smart_evcoro_lua_api.h"
#include "lua_mappoi.h"

static int _getProgName(lua_State *L)
{
	char            buff[32] = {};
	const char      *prog = NULL;

	prog = GetProgName(buff, sizeof(buff));

	lua_pushstring(L, prog);

	return 1;
}

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
	lua_register(L, "app_lua_get_head_data", app_lua_get_head_data);
	lua_register(L, "app_lua_get_body_data", app_lua_get_body_data);
	lua_register(L, "app_lua_get_path_data", app_lua_get_path_data);
	lua_register(L, "app_lua_get_uri_args", app_lua_get_uri_args);
	lua_register(L, "app_lua_get_recv_data", app_lua_get_recv_data);
	lua_register(L, "app_lua_add_send_data", app_lua_add_send_data);
	lua_register(L, "app_lua_mapinit", app_lua_mapinit);
	lua_register(L, "app_lua_convert", app_lua_convert);
	lua_register(L, "app_lua_reverse", app_lua_reverse);
	lua_register(L, "app_lua_ifmatch", app_lua_ifmatch);

	lua_register(L, "getprogname", _getProgName);

#ifdef GOBY
	lua_register(L, "getpoi", getpoi);
#endif
	// lua_register(L, "luakv_cmd", luakv_run);
	// lua_register(L, "luakv_ask", luakv_iterfactory);
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

/*
 * 函 数:smart_vms_init
 * 功 能:初始化虚拟机
 * 参 数:user 指向线程的附属数据结构, task指向任务
 * 返回值: 成功返回０, 失败返回非０
 * 修 改:添加注释 程少远 2015/05/12
 */
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

int smart_vms_call_ext(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	int             error = 0;
	lua_State       **L = VMS;

	lua_getglobal(*L, "app_call_1");
	lua_pushboolean(*L, task->last);
	lua_pushinteger(*L, task->sfd);
	error = lua_pcall(*L, 2, 0, 0);

	if (error) {
		assert(*L);
		x_printf(E, "%s", lua_tostring(*L, -1));
		lua_pop(*L, 1);
	}

	return error;
}

