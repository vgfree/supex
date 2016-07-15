#include <string.h>
#include <stdlib.h>

#include "base/utils.h"
#include "major/alive_api.h"
#include "minor/sniff_api.h"
#include "alive_cpp_api.h"

#include "sniff_evcoro_lua_api.h"



extern struct sniff_cfg_list g_sniff_cfg_list;


int alive_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	printf("size %d data %s\n", task->size, task->data);

	ALIVE_WORKER_PTHREAD    *p_alive_worker = (ALIVE_WORKER_PTHREAD *)user;

	if (task->size == 0) {
		// TODO
		// x_printf
		return -1;
	}

	if (task->size > MAX_SNIFF_DATA_SIZE) {
		// TODO
		// x_printf
		return -1;
	}

	struct sniff_task_node sniff_task = {};

	sniff_task.sfd = task->sfd;
	sniff_task.type = task->type;
	sniff_task.origin = task->origin;
	sniff_task.func = (SNIFF_VMS_FCB)sniff_vms_call;
	sniff_task.last = false;
	sniff_task.stamp = time(NULL);
	sniff_task.size = task->size;
	memcpy(sniff_task.data, (const char *)task->data, task->size);

	alive_send_data(true, task->cid, task->sfd, "good", 5);

	return 0;
}

int alive_vms_online(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	return 0;
}

int alive_vms_offline(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	return 0;
}

