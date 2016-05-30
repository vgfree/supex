#include <assert.h>
#include "lj_sniff_util.h"

int sniff_vms_cntl(void *user, union virtual_system **VMS, struct sniff_task_node *task)
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


int sniff_vms_exit(void *user, union virtual_system **VMS, struct sniff_task_node *task)
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


int sniff_vms_rfsh(void *user, union virtual_system **VMS, struct sniff_task_node *task)
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


int sniff_vms_sync(void *user, union virtual_system **VMS, struct sniff_task_node *task)
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
int sniff_vms_gain(void *user, union virtual_system **VMS, struct sniff_task_node *task)
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


int sniff_vms_call(void *user, union virtual_system **VMS, struct sniff_task_node *task)
{
	int error = 0;
	lua_State       **L = VMS;
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	time_t                  delay = time(NULL) - task->stamp;

	x_printf(S, "channel %d\t|task <shift> %d\t<come> %ld\t<delay> %ld",
		p_sniff_worker->genus, task->base.shift, task->stamp, delay);

	x_printf(D, "%s", task->data);

	if (delay >= OVERLOOK_DELAY_LIMIT) {
		x_printf(W, "overlook one task");
		return -1;
	}
	x_printf(S, "task <thread> %d\t<shift> %d\t<come> %d\t<delay> %d\n",
		task->thread_id, task->base.shift, task->stamp, time(NULL) - task->stamp);
	lua_getglobal(*L, "app_call_all");
	lua_pushboolean(*L, task->last);
	lua_pushlstring(*L, (const char *)task->data, task->size);
	error = lua_pcall(*L, 2, 0, 0);
	if (error) {
		assert(*L);
		x_printf(E, "%s", lua_tostring(*L, -1));
		lua_pop(*L, 1);
	}
	return error;
}


int sniff_vms_exec(void *user, union virtual_system **VMS, struct sniff_task_node *task)
{
	int error = 0;
	lua_State       **L = VMS;
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	time_t                  delay = time(NULL) - task->stamp;

	x_printf(S, "channel %d\t|task <shift> %d\t<come> %ld\t<delay> %ld",
		p_sniff_worker->genus, task->base.shift, task->stamp, delay);

	x_printf(D, "%s", task->data);

	if (delay >= OVERLOOK_DELAY_LIMIT) {
		x_printf(W, "overlook one task");
		return -1;
	}

	x_printf(S, "task <thread> %d\t<shift> %d\t<come> %d\t<delay> %d\n",
		task->thread_id, task->base.shift, task->stamp, time(NULL) - task->stamp);
	lua_getglobal(*L, "app_call_one");
	lua_pushboolean(*L, task->last);
	lua_pushlstring(*L, (const char *)task->data, task->size);
	error = lua_pcall(*L, 2, 0, 0);
	if (error) {
		assert(*L);
		x_printf(E, "%s", lua_tostring(*L, -1));
		lua_pop(*L, 1);
	}
	return error;
}

