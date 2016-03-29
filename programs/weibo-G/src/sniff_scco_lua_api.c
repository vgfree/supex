#include <assert.h>

#include "match.h"
#include "sniff_api.h"
#include "sniff_scco_lua_api.h"
#include "lua_scco.h"

extern int async_http(lua_State *L);

static void _vms_erro(lua_State **L)
{
	assert(*L);
	x_printf(E, "%s\n", lua_tostring(*L, -1));
	lua_pop(*L, 1);
}

static int _vms_cntl(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	struct msg_info *msg = task->data;

	assert(msg);

	lua_getglobal(*L, "app_cntl");
	lua_pushboolean(*L, last);
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
	return lua_pcall(*L, 3, 0, 0);
}

int sniff_vms_cntl(void *user, void *task, int step)
{
	return sniff_for_batch_vm(user, task, step, _vms_cntl, _vms_erro);
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
	lua_register(L, "lua_default_switch", lua_scco_switch);
	lua_register(L, "app_lua_mapinit", app_lua_mapinit);
	lua_register(L, "app_lua_convert", app_lua_convert);
	lua_register(L, "app_lua_reverse", app_lua_reverse);
	lua_register(L, "app_lua_ifmatch", app_lua_ifmatch);
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
			fprintf(stderr, "%s\n", lua_tostring(L, -1));
			lua_pop(L, 1);
			exit(EXIT_FAILURE);
		}
	}
	/*app init*/
	error = luaL_dofile(L, "sniff_lua/core/start.lua");

	if (error) {
		fprintf(stderr, "%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		exit(EXIT_FAILURE);
	}

	return L;
}

static int _vms_init(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	if (*L != NULL) {
		x_printf(S, "No need to init LUA VM!\n");
		return 0;
	}

	*L = _vms_new();
	assert(*L);
	lua_getglobal(*L, "app_scco_init");
	lua_pushinteger(*L, S);
	// lua_pushinteger(*L, task->index);
	lua_pushinteger(*L, 0);
	return lua_pcall(*L, 2, 0, 0);
}

int sniff_vms_init(void *user, void *task, int step)
{
	int error = sniff_for_batch_vm(user, task, step, _vms_init, _vms_erro);

	if (error) {
		exit(EXIT_FAILURE);
	}

	return error;
}

static int _vms_exit(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	int error = 0;

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

int sniff_vms_exit(void *user, void *task, int step)
{
	int error = sniff_for_batch_vm(user, task, step, _vms_exit, _vms_erro);

	x_printf(S, "exit one batch LUA!\n");
	return error;
}

static int _vms_rfsh(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	lua_getglobal(*L, "app_rfsh");
	lua_pushboolean(*L, last);
	lua_pushinteger(*L, task->sfd);
	return lua_pcall(*L, 2, 0, 0);
}

int sniff_vms_rfsh(void *user, void *task, int step)
{
	return sniff_for_batch_vm(user, task, step, _vms_rfsh, _vms_erro);
}

static int _vms_sync(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	lua_getglobal(*L, "app_push");
	lua_pushboolean(*L, last);
	lua_pushinteger(*L, task->sfd);
	return lua_pcall(*L, 2, 0, 0);
}

int sniff_vms_sync(void *user, void *task, int step)
{
	return sniff_for_batch_vm(user, task, step, _vms_sync, _vms_erro);
}

/*=============================================================*/
static int _vms_gain(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	lua_getglobal(*L, "app_pull");
	lua_pushboolean(*L, last);
	lua_pushinteger(*L, task->sfd);
	return lua_pcall(*L, 2, 0, 0);
}

int sniff_vms_gain(void *user, void *task, int step)
{
	return sniff_for_alone_vm(user, task, step, _vms_gain, _vms_erro);
}

static int _vms_call(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	lua_getglobal(*L, "app_call_all");
	lua_pushboolean(*L, last);
	lua_pushlstring(*L, (const char *)task->data, task->size);
	return lua_pcall(*L, 2, 0, 0);
}

int sniff_vms_call(void *user, void *task, int step)
{
	return sniff_for_alone_vm(user, task, step, _vms_call, _vms_erro);
}

static int _vms_exec(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	lua_getglobal(*L, "app_call_one");
	lua_pushboolean(*L, last);
	lua_pushlstring(*L, (const char *)task->data, task->size);
	return lua_pcall(*L, 2, 0, 0);
}

int sniff_vms_exec(void *user, void *task, int step)
{
	return sniff_for_alone_vm(user, task, step, _vms_exec, _vms_erro);
}

int sniff_vms_idle(void *user, void *task, int step)
{
	return 0;
}

