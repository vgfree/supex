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
	tlpool_t                *tlpool = user;
	int                     idx = tlpool_get_thread_index(tlpool);
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[idx];
	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	unsigned                size = cache_data_length(&p_node->mdl_recv.cache);
	struct http_status      *p_hst = &p_node->mdl_recv.parse.http_info.hs;

	struct sniff_task_node sniff_task = {};

	sniff_task.sfd = task->sfd;
	sniff_task.type = task->type;
	sniff_task.origin = task->origin;
	sniff_task.func = (SNIFF_VMS_FCB)sniff_vms_call;	// fix to use xxxx;
	sniff_task.last = false;				// FIXME
	sniff_task.stamp = time(NULL);
	sniff_task.size = p_hst->body_size;
	memcpy(sniff_task.data, (const char *)(p_buf + p_hst->body_offset), p_hst->body_size);

	g_sniff_cfg_list.task_report(p_swift_worker->mount, &sniff_task);
	return 0;
}

int swift_vms_exec(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	tlpool_t                *tlpool = user;
	int                     idx = tlpool_get_thread_index(tlpool);
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[idx];
	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	unsigned                size = cache_data_length(&p_node->mdl_recv.cache);
	struct http_status      *p_hst = &p_node->mdl_recv.parse.http_info.hs;

	struct sniff_task_node sniff_task = {};

	sniff_task.sfd = task->sfd;
	sniff_task.type = task->type;
	sniff_task.origin = task->origin;
	sniff_task.func = sniff_vms_exec;	// fix to use xxxx;
	sniff_task.last = false;		// FIXME
	sniff_task.stamp = time(NULL);
	sniff_task.size = p_hst->body_size;
	memcpy(sniff_task.data, (const char *)(p_buf + p_hst->body_offset), p_hst->body_size);

	g_sniff_cfg_list.task_report(p_swift_worker->mount, &sniff_task);
	return 0;
}

