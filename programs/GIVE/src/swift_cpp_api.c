#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "swift_api.h"
#include "sniff_api.h"
#include "swift_cpp_api.h"
#include "apply_def.h"

#include "sniff_evuv_cpp_api.h"

extern struct sniff_cfg_list g_sniff_cfg_list;

int swift_vms_init(void *W)
{
	/*
	 *   SWIFT_WORKER_PTHREAD *p_swift_worker = (SWIFT_WORKER_PTHREAD *)W;
	 *   struct swift_task_node *swift_task = &p_swift_worker->task;
	 *
	 *   struct sniff_task_node sniff_task = {};
	 *   sniff_task.sfd = swift_task->sfd;
	 *   sniff_task.type = swift_task->type;
	 *   sniff_task.origin = swift_task->origin;
	 *   sniff_task.func = g_sniff_cfg_list.vmsys_init;
	 *   sniff_task.last = false;//FIXME
	 *   sniff_task.size = 0;
	 *
	 *   sniff_all_task_hit( (SNIFF_WORKER_PTHREAD *)p_swift_worker->mount, &sniff_task );
	 */
	return 0;
}

int swift_vms_call(void *W)
{
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)W;
	struct swift_task_node  *swift_task = &p_swift_worker->task;
	struct data_node        *p_node = get_pool_addr(swift_task->sfd);
	char                    *p_buf = p_node->recv.buf_addr;
	struct http_status      *p_hst = &p_node->http_info.hs;

	if (p_hst->body_size == 0) {
		// TODO
		// x_printf
		return -1;
	}

	if (p_hst->body_size > MAX_SNIFF_DATA_SIZE) {
		// TODO
		// x_printf
		return -1;
	}

	struct sniff_task_node sniff_task = {};

	sniff_task.sfd = swift_task->sfd;
	sniff_task.type = swift_task->type;
	sniff_task.origin = swift_task->origin;
	sniff_task.func = (SUPEX_TASK_CALLBACK)sniff_vms_call;	// fix to use xxxx;
	sniff_task.last = false;				// FIXME
	sniff_task.stamp = time(NULL);
	sniff_task.size = p_hst->body_size;
	memcpy(sniff_task.data, (const char *)(p_node->recv.buf_addr + p_hst->body_offset), p_hst->body_size);

	struct mount_info *mnt = (struct mount_info *)p_swift_worker->mount;

	while (mnt) {
		sniff_one_task_hit(mnt->list, &sniff_task);
		mnt->list = mnt->list->next;
		mnt = mnt->next;
	}

	return 0;
}

