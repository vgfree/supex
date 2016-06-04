#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "major/swift_api.h"
#include "minor/sniff_api.h"
#include "swift_cpp_api.h"

#include "sniff_evcoro_lua_api.h"

extern struct sniff_cfg_list g_sniff_cfg_list;

SNIFF_WORKER_PTHREAD *get_idle_thread(SNIFF_WORKER_PTHREAD *p_list)
{
	SNIFF_WORKER_PTHREAD    *p_idle = p_list;
	int                     idle = p_idle->thave;
	SNIFF_WORKER_PTHREAD    *p_temp = p_list->next;

	while (p_temp != p_list) {
		int temp = p_temp->thave;

		if (temp < idle) {
			p_idle = p_temp;
			idle = temp;
		}

		p_temp = p_temp->next;
	}

	return p_idle;
}

int swift_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)user;
	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	unsigned                size = cache_data_length(&p_node->mdl_recv.cache);
	struct http_status      *p_hst = &p_node->mdl_recv.parse.http_info.hs;

	struct sniff_task_node sniff_task = {};

	sniff_task.sfd = task->sfd;
	sniff_task.type = task->type;
	sniff_task.origin = task->origin;
	sniff_task.func = (SNIFF_VMS_FCB)sniff_vms_call;	// fix to use xxxx;
	sniff_task.last = false;		// FIXME
	sniff_task.stamp = time(NULL);
	sniff_task.size = p_hst->body_size;
	memcpy(sniff_task.data, (const char *)(p_buf + p_hst->body_offset), p_hst->body_size);

#if 0
	sniff_task.thread_id = ((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount)->thread_id;
	sniff_one_task_hit((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount, &sniff_task);
	p_swift_worker->mount = ((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount)->next;
#else
	SNIFF_WORKER_PTHREAD *p_sniff_worker = get_idle_thread((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount);
	sniff_task.thread_id = p_sniff_worker->pid;
	sniff_one_task_hit(p_sniff_worker, &sniff_task);
	p_swift_worker->mount = p_sniff_worker->next;
#endif
	return 0;
}

int swift_vms_exec(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)user;
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

#if 0
	sniff_task.thread_id = ((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount)->thread_id;
	sniff_one_task_hit((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount, &sniff_task);
	p_swift_worker->mount = ((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount)->next;
#else
	SNIFF_WORKER_PTHREAD *p_sniff_worker = get_idle_thread((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount);
	sniff_task.thread_id = p_sniff_worker->pid;
	sniff_one_task_hit(p_sniff_worker, &sniff_task);
	p_swift_worker->mount = p_sniff_worker->next;
#endif
	return 0;
}

