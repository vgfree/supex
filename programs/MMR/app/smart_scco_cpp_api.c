#include <string.h>
#include <stdlib.h>

#include "smart_scco_cpp_api.h"
#include "sniff_line_lua_api.h"
#include "utils.h"
#include "smart_api.h"
#include "swift_api.h"
#include "sniff_api.h"

extern SWIFT_WORKER_PTHREAD     *g_swift_worker_pthread;
extern struct swift_cfg_list    g_swift_cfg_list;
extern struct sniff_cfg_list    g_sniff_cfg_list;

static int _vms_init(lua_State **L, int last, struct smart_task_node *task, long S)
{
	return 0;
}

int smart_vms_init(void *user, void *task, int step)
{
	return smart_for_batch_vm(user, task, step, _vms_init);
}

static int _vms_gain(lua_State **L, int last, struct smart_task_node *task, long S)
{
	struct smart_task_node *p_task = (struct smart_task_node *)task;

	if (last != true) {
		return 0;
	}

	int all = 1;

	int     m = g_swift_cfg_list.file_info.worker_counts;
	int     i = time(NULL) % m;
	{
		SWIFT_WORKER_PTHREAD *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[i];

		struct sniff_task_node sniff_task = {};

		sniff_task.sfd = p_task->sfd;
		sniff_task.type = BIT8_TASK_TYPE_ALONE;
		sniff_task.origin = p_task->origin;
		sniff_task.func = (SUPEX_TASK_CALLBACK)sniff_vms_gain;	// fix to use xxxx;
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

int smart_vms_gain(void *user, void *task, int step)
{
	return smart_for_alone_vm(user, task, step, _vms_gain);
}

static int _vms_sync(lua_State **L, int last, struct smart_task_node *task, long S)
{
	struct smart_task_node *p_task = (struct smart_task_node *)task;

	if (last != true) {
		return 0;
	}

#ifdef OPEN_SCCO
	int all = g_swift_cfg_list.file_info.worker_counts * g_sniff_cfg_list.file_info.worker_counts * g_sniff_cfg_list.file_info.sharer_counts;
#else
	int all = g_swift_cfg_list.file_info.worker_counts * g_sniff_cfg_list.file_info.worker_counts;
#endif

	int     i = 0;
	int     m = g_swift_cfg_list.file_info.worker_counts;

	for (i = 0; i < m; i++) {
		SWIFT_WORKER_PTHREAD *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[i];

		struct sniff_task_node sniff_task = {};

		sniff_task.sfd = p_task->sfd;
		sniff_task.type = BIT8_TASK_TYPE_WHOLE;
		sniff_task.origin = p_task->origin;
		sniff_task.func = (SUPEX_TASK_CALLBACK)sniff_vms_sync;	// fix to use xxxx;
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

int smart_vms_sync(void *user, void *task, int step)
{
	return smart_for_batch_vm(user, task, step, _vms_sync);
}

static int _vms_monitor(lua_State **L, int last, struct smart_task_node *task, long S)
{
	struct smart_task_node *p_task = (struct smart_task_node *)task;

	if (last != true) {
		return 0;
	}

	int     i = 0;
	int     m = g_swift_cfg_list.file_info.worker_counts;

	for (i = 0; i < m; i++) {
		SWIFT_WORKER_PTHREAD *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[i];

		struct sniff_task_node sniff_task = {};

		sniff_task.sfd = p_task->sfd;
		sniff_task.type = BIT8_TASK_TYPE_WHOLE;
		sniff_task.origin = p_task->origin;
		sniff_task.func = (SUPEX_TASK_CALLBACK)sniff_vms_monitor;	// fix to use xxxx;
		sniff_task.last = (i == (m - 1)) ? true : false;		// FIXME
		sniff_task.stamp = time(NULL);
		sniff_task.size = 0;

		sniff_all_task_hit((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount, &sniff_task);
	}

	return 0;
}

int smart_vms_monitor(void *user, void *task, int step)
{
	return smart_for_batch_vm(user, task, step, _vms_monitor);
}

