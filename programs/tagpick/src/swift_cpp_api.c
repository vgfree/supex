#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "major/swift_api.h"
#include "minor/sniff_api.h"
#include "swift_cpp_api.h"

#include "sniff_evcoro_lua_api.h"

extern struct sniff_cfg_list g_sniff_cfg_list;

int swift_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)user;
	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	unsigned                size = cache_data_length(&p_node->mdl_recv.cache);
	struct http_status      *p_hst = &p_node->mdl_recv.parse.http_info.hs;

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
	sniff_task.func = (SNIFF_VMS_FCB)sniff_vms_call;	// fix to use xxxx;
	sniff_task.last = false;				// FIXME
	sniff_task.stamp = time(NULL);
	// sniff_task.size = p_hst->body_size;
	// memcpy(sniff_task.data, (const char *)(p_node->recv.buf_addr + p_hst->body_offset), p_hst->body_size);
	sniff_task.size = size;
	memcpy(sniff_task.data, (const char *)p_buf, size);

	SNIFF_WORKER_PTHREAD *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)p_swift_worker->mount;
	sniff_one_task_hit(p_sniff_worker, &sniff_task);
	p_swift_worker->mount = p_sniff_worker->next;
	return 0;
}

