#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "swift_api.h"
#include "sniff_api.h"
#include "swift_cpp_api.h"

#ifdef OPEN_SCCO
  #include "sniff_scco_lua_api.h"
#else
  #include "sniff_line_lua_api.h"
#endif

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

SNIFF_WORKER_PTHREAD *get_idle_thread(SNIFF_WORKER_PTHREAD *p_list)
{
	SNIFF_WORKER_PTHREAD    *p_idle = p_list;
	AO_T                    idle = AO_GET(&p_idle->thave);
	SNIFF_WORKER_PTHREAD    *p_temp = p_list->next;

	while (p_temp != p_list) {
		AO_T temp = AO_GET(&p_temp->thave);

		if (temp < idle) {
			p_idle = p_temp;
			idle = temp;
		}

		p_temp = p_temp->next;
	}

	return p_idle;
}

int swift_vms_call(void *W)
{
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)W;
	struct swift_task_node  *swift_task = &p_swift_worker->task;

	struct sniff_task_node sniff_task = {};

	sniff_task.sfd = swift_task->sfd;
	sniff_task.type = swift_task->type;
	sniff_task.origin = swift_task->origin;
	sniff_task.func = (SUPEX_TASK_CALLBACK)sniff_vms_call;	// fix to use xxxx;
	sniff_task.last = false;				// FIXME
	sniff_task.stamp = time(NULL);
	get_cache_data(swift_task->sfd, sniff_task.data, &sniff_task.size);

#if 0
	sniff_task.thread_id = ((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount)->thread_id;
	sniff_one_task_hit((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount, &sniff_task);
	p_swift_worker->mount = ((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount)->next;
#else
	SNIFF_WORKER_PTHREAD *p_sniff_worker = get_idle_thread((SNIFF_WORKER_PTHREAD *)p_swift_worker->mount);
	sniff_task.thread_id = p_sniff_worker->thread_id;
	sniff_one_task_hit(p_sniff_worker, &sniff_task);
	p_swift_worker->mount = p_sniff_worker->next;
#endif
	return 0;
}

