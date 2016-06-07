#include <string.h>
#include <stdlib.h>

#include "smart_evcoro_cpp_api.h"
#include "sniff_evcoro_lua_api.h"
#include "base/utils.h"
#include "major/smart_api.h"
#include "major/swift_api.h"
#include "minor/sniff_api.h"

extern SWIFT_WORKER_PTHREAD     *g_swift_worker_pthread;
extern struct swift_cfg_list    g_swift_cfg_list;
extern struct sniff_cfg_list    g_sniff_cfg_list;

int smart_vms_gain_ext(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	if (task->last != true) {
		return 0;
	}

	int all = 1;

	int     m = g_swift_cfg_list.file_info.worker_counts;
	int     i = time(NULL) % m;
	{
		SWIFT_WORKER_PTHREAD *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[i];

		struct sniff_task_node sniff_task = {};

		sniff_task.sfd = task->sfd;
		sniff_task.type = BIT8_TASK_TYPE_ALONE;
		sniff_task.origin = task->origin;
		sniff_task.func = (SNIFF_VMS_FCB)sniff_vms_gain_ext;	// fix to use xxxx;
		sniff_task.last = true;					// FIXME
		sniff_task.stamp = time(NULL);
		sniff_task.size = sizeof(int *);
		long *addr = sniff_task.data;
		*addr = &all;

		sniff_one_task_hit((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount, &sniff_task);
	}

	while (all != 0) {
		usleep(1000);
	}

	return 0;
}


int smart_vms_sync_ext(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	if (task->last != true) {
		return 0;
	}

	int all = g_swift_cfg_list.file_info.worker_counts * g_sniff_cfg_list.file_info.tasker_counts;

	int     i = 0;
	int     m = g_swift_cfg_list.file_info.worker_counts;

	for (i = 0; i < m; i++) {
		SWIFT_WORKER_PTHREAD *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[i];

		struct sniff_task_node sniff_task = {};

		sniff_task.sfd = task->sfd;
		sniff_task.type = BIT8_TASK_TYPE_WHOLE;
		sniff_task.origin = task->origin;
		sniff_task.func = (SNIFF_VMS_FCB)sniff_vms_sync_ext;	// fix to use xxxx;
		sniff_task.last = (i == (m - 1)) ? true : false;	// FIXME
		sniff_task.stamp = time(NULL);
		sniff_task.size = sizeof(int *);
		long *addr = sniff_task.data;
		*addr = &all;

		sniff_all_task_hit((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount, &sniff_task);
	}

	while (all != 0) {
		usleep(1000);
	}

	return 0;
}


int smart_vms_monitor_ext(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	if (task->last != true) {
		return 0;
	}

	int     i = 0;
	int     m = g_swift_cfg_list.file_info.worker_counts;

	for (i = 0; i < m; i++) {
		SWIFT_WORKER_PTHREAD *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[i];

		struct sniff_task_node sniff_task = {};

		sniff_task.sfd = task->sfd;
		sniff_task.type = BIT8_TASK_TYPE_WHOLE;
		sniff_task.origin = task->origin;
		sniff_task.func = (SNIFF_VMS_FCB)sniff_vms_monitor;	// fix to use xxxx;
		sniff_task.last = (i == (m - 1)) ? true : false;		// FIXME
		sniff_task.stamp = time(NULL);
		sniff_task.size = 0;

		sniff_all_task_hit((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount, &sniff_task);
	}

	return 0;
}

