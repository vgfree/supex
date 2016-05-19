#include <assert.h>

#include "major/smart_api.h"
#include "lua_expand/lj_c_coro.h"
#include "lua_expand/lua_link.h"
// #include "luakvcore.h"
#include "smart_evcoro_lua_api.h"

#ifdef OPEN_ZOOKEEPER
  #include "lua_zk.h"
#endif

extern int async_http_evcoro(lua_State *L);

extern int lua_evcoro_switch(lua_State *L);

extern int app_lua_get_head_data(lua_State *L);

extern int app_lua_get_body_data(lua_State *L);

extern int app_lua_get_path_data(lua_State *L);

extern int app_lua_get_uri_args(lua_State *L);

extern int app_lua_add_send_data(lua_State *L);

int smart_vms_cntl(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	int             error = 0;
	struct msg_info *msg = task->data;
	lua_State       **L = VMS;

	assert(msg);

	lua_getglobal(*L, "app_cntl");
	lua_pushboolean(*L, task->last);
	lua_pushstring(*L, msg->data);
	switch (msg->opt)
	{
		case 'l':
			lua_pushstring(*L, "insmod");
			break;

		case 'f':
			lua_pushstring(*L, "rmmod");
			break;

		case 'o':
			lua_pushstring(*L, "open");
			break;

		case 'c':
			lua_pushstring(*L, "close");
			break;

		case 'd':
			lua_pushstring(*L, "delete");
			break;

		default:
			x_printf(S, "Error msmq opt!\n");
			return 0;
	}
	error = lua_pcall(*L, 3, 0, 0);
	if (error) {
		assert(*L);
		x_printf(E, "%s", lua_tostring(*L, -1));
		lua_pop(*L, 1);
	}
	return error;
}


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
	/*
	lua_register(L, "app_lua_get_head_data", app_lua_get_head_data);
	lua_register(L, "app_lua_get_body_data", app_lua_get_body_data);
	lua_register(L, "app_lua_get_path_data", app_lua_get_path_data);
	lua_register(L, "app_lua_get_uri_args", app_lua_get_uri_args);
	lua_register(L, "app_lua_get_recv_buf", app_lua_get_recv_buf);
	lua_register(L, "app_lua_add_send_data", app_lua_add_send_data);
	*/
	lua_register(L, "search_kvhandle", search_kvhandle);
	// lua_register(L, "luakv_cmd", luakv_run);
	// lua_register(L, "luakv_ask", luakv_iterfactory);
#ifdef OPEN_ZOOKEEPER
	lua_register(L, "zk_get_read_dn", zk_get_read_dn);
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


int smart_vms_exit(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	int error = 0;
	lua_State       **L = VMS;
	x_printf(S, "exit one batch LUA!\n");

	lua_getglobal(*L, "app_exit");
	error = lua_pcall(*L, 0, 0, 0);

	if (error) {
		x_printf(E, "%s\n", lua_tostring(*L, -1));
		lua_pop(*L, 1);
	}

	lua_close(*L);
	*L = NULL;
	return 0;	/*must return 0*/
}


int smart_vms_rfsh(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	int error = 0;
	lua_State       **L = VMS;
	lua_getglobal(*L, "app_rfsh");
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


int smart_vms_sync(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	int error = 0;
	lua_State       **L = VMS;
	lua_getglobal(*L, "app_push");
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

/*=============================================================*/
int smart_vms_gain(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	int error = 0;
	lua_State       **L = VMS;
	lua_getglobal(*L, "app_pull");
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


int smart_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	int error = 0;
	lua_State       **L = VMS;
	lua_getglobal(*L, "app_call_all");
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


int smart_vms_exec(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	int error = 0;
	lua_State       **L = VMS;
	lua_getglobal(*L, "app_call_one");
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

