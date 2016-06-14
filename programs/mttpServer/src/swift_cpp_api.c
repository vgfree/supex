#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "swift_api.h"
#include "sniff_api.h"
#include "swift_cpp_api.h"
#include "apply_def.h"
#include "mttp.h"

#include "sniff_evuv_cpp_api.h"

extern struct sniff_cfg_list g_sniff_cfg_list;


int swift_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	tlpool_t	*tlpool = user;
	int idx = tlpool_get_thread_index(tlpool);
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[idx];
	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *p_buf = p_node->recv.buf_addr;
	struct mttp_status      *p_hst = &p_node->mttp_info.mt;

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

	sniff_task.sfd = task->sfd;
	sniff_task.type = task->type;
	sniff_task.origin = task->origin;
	sniff_task.func = (SUPEX_TASK_CALLBACK)sniff_vms_call;	// fix to use xxxx;
	sniff_task.last = false;				// FIXME
	sniff_task.stamp = time(NULL);
	sniff_task.size = p_hst->body_size;

	memcpy(sniff_task.data, (const char *)(p_node->recv.buf_addr + p_hst->dosize + p_hst->headlen), p_hst->body_size);

	sniff_one_task_hit(p_swift_worker->mount, &sniff_task);

	return 0;
}

