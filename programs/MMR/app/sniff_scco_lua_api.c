#include <assert.h>

#include "match.h"
#include "sniff_api.h"
#include "sniff_scco_lua_api.h"

extern int async_http(lua_State *L);

extern int app_lua_get_head_data(lua_State *L);

extern int app_lua_get_body_data(lua_State *L);

extern int app_lua_get_path_data(lua_State *L);

extern int app_lua_get_recv_data(lua_State *L);

extern int app_lua_get_uri_args(lua_State *L);

extern int app_lua_add_send_data(lua_State *L);

#ifdef OPEN_TOPO
extern int cal_poi_by_line(lua_State *L);

extern int cal_front_road_by_road(lua_State *L);
#endif
static int _getProgName(lua_State *L)
{
	char            buff[32] = {};
	const char      *prog = NULL;

	prog = GetProgName(buff, sizeof(buff));

	lua_pushstring(L, prog);

	return 1;
}

static void _vms_erro(lua_State **L)
{
	assert(*L);
	x_printf(E, "%s\n", lua_tostring(*L, -1));
	lua_pop(*L, 1);
}

/******************************************************************************************/
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
			x_printf(S, "Error msmq opt!");
			return 0;
	}
	lua_pushinteger(*L, msg->mode - '0');
	return lua_pcall(*L, 4, 0, 0);
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
	lua_register(L, "app_lua_get_head_data", app_lua_get_head_data);
	lua_register(L, "app_lua_get_body_data", app_lua_get_body_data);
	lua_register(L, "app_lua_get_path_data", app_lua_get_path_data);
	lua_register(L, "app_lua_get_recv_data", app_lua_get_recv_data);
	lua_register(L, "app_lua_get_uri_args", app_lua_get_uri_args);
	lua_register(L, "app_lua_add_send_data", app_lua_add_send_data);
	lua_register(L, "app_lua_mapinit", app_lua_mapinit);
	lua_register(L, "app_lua_convert", app_lua_convert);
	lua_register(L, "app_lua_reverse", app_lua_reverse);
	lua_register(L, "app_lua_ifmatch", app_lua_ifmatch);

	lua_register(L, "getprogname", _getProgName);

#ifdef OPEN_TOPO
	lua_register(L, "cal_poi_by_line", cal_poi_by_line);
	lua_register(L, "cal_front_road_by_road", cal_front_road_by_road);
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

		error = luaL_dofile(L, "lua/core/init.lua");

		if (error) {
			x_printf(E, "%s", lua_tostring(L, -1));
			lua_pop(L, 1);
			exit(EXIT_FAILURE);
		}
	}
	/*app init*/
	error = luaL_dofile(L, "lua/core/start.lua");

	if (error) {
		x_printf(E, "%s", lua_tostring(L, -1));
		lua_pop(L, 1);
		exit(EXIT_FAILURE);
	}

	lua_getglobal(L, "app_type");
	lua_pcall(L, 0, 0, 0);
	return L;
}

static int _vms_init(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	if (*L != NULL) {
		x_printf(S, "No need to init LUA VM!");
		return 0;
	}

	*L = _vms_new();
	assert(*L);
	lua_getglobal(*L, "app_scco_init");
	lua_pushinteger(*L, S);
	return lua_pcall(*L, 1, 0, 0);
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
		x_printf(E, "%s", lua_tostring(*L, -1));
		lua_pop(*L, 1);
	}

	lua_close(*L);
	*L = NULL;
	return 0;	/*must return 0*/
}

int sniff_vms_exit(void *user, void *task, int step)
{
	int error = sniff_for_batch_vm(user, task, step, _vms_exit, _vms_erro);

	x_printf(S, "exit one batch LUA!");
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
	long    *addr = task->data;
	int     *all = *addr;

	ATOMIC_F_SUB(all, 1);

	lua_getglobal(*L, "app_push");
	lua_pushboolean(*L, last);
	lua_pushinteger(*L, task->sfd);
	return lua_pcall(*L, 2, 0, 0);
}

int sniff_vms_sync(void *user, void *task, int step)
{
	return sniff_for_batch_vm(user, task, step, _vms_sync, _vms_erro);
}

/*
 * 函 数:_vms_monitor
 * 功 能:监控系统的回调函数
 * 参 数:L 虚拟机，last是否为最后一个处理task的线程，　task指向任务
 * 返回值:返回调用lua中app_monitor的返回值
 * 修 改:添加新函数　　程少远 2015/05/12
 */
static int _vms_monitor(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	lua_getglobal(*L, "app_monitor");
	return lua_pcall(*L, 0, 0, 0);
}

/*
 * 函 数:sniff_vms_monitor
 * 功 能:监控系统的回调函数
 * 参 数:user 指向工作线程的附属数据结构 task指向任务
 * 返回值:返回调用sniff_for_alone_vms的返回值
 * 修 改:添加新函数　程少远 2015/05/12
 */
int sniff_vms_monitor(void *user, void *task, int step)
{
	return sniff_for_batch_vm(user, task, step, _vms_monitor, _vms_erro);
}

/*=============================================================*/
static int _vms_gain(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	lua_getglobal(*L, "app_pull");
	lua_pushboolean(*L, last);
	lua_pushinteger(*L, task->sfd);
	int ret = lua_pcall(*L, 2, 0, 0);

	long    *addr = task->data;
	int     *all = *addr;
	ATOMIC_F_SUB(all, 1);

	return ret;
}

int sniff_vms_gain(void *user, void *task, int step)
{
	return sniff_for_alone_vm(user, task, step, _vms_gain, _vms_erro);
}

static int _vms_call_1(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	lua_getglobal(*L, "app_call_1");
	lua_pushboolean(*L, last);
	lua_pushinteger(*L, task->sfd);
	return lua_pcall(*L, 2, 0, 0);
}

static int _vms_call_2(lua_State **L, int last, struct sniff_task_node *task, long S)
{
	lua_getglobal(*L, "app_call_2");
	lua_pushboolean(*L, last);
	lua_pushlstring(*L, (const char *)task->data, task->size);
	return lua_pcall(*L, 2, 0, 0);
}

int sniff_vms_call(void *user, void *task, int step)
{
	return sniff_for_alone_vm(user, task, step, _vms_call_2, _vms_erro);
}

