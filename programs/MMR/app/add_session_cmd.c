//
//  add_session_cmd.c
//  supex
//
//  Created by 周凯 on 15/8/6.
//  Copyright (c) 2015年 zk. All rights reserved.
//

#include "add_session_cmd.h"
#include "sniff_api.h"
#include "swift_evcb.h"
#include "swift_api.h"
#include "swift_evcb.h"

ShmQueueT g_tasks_shmqueue = NULL;

static bool help(void *user, void *data)
{
	bool flag = false;

	static char helpinfo[] = "[help] to list what has been supported commands.\n"
#if defined(USE_MEMHOOK)
		"[memblocks] to check how much blocks that were allocated in heap.\n"
		"[memdetails] to print information about memory that were allocated in heap.\n"
#endif
		"[tasks] to check how much tasks that belong to parters.\n";

	struct session_task *service = (struct session_task *)data;

	if (!service) {
		return false;
	}

	flag = session_response_clnt(service->fd, SESSION_IO_TIMEOUT, "%s", helpinfo);

	return flag;
}

static bool tasks(void *user, void *data)
{
	bool                    flag = false;
	int                     taskid = 0;
	struct swift_task_node  *task = NULL;
	struct session_task     *service = NULL;
	SWIFT_WORKER_PTHREAD    *p_swift_worker = NULL;

	assert(user);
	assert(data);

	task = (struct swift_task_node *)data;
	p_swift_worker = (SWIFT_WORKER_PTHREAD *)user;
	service = (struct session_task *)task->data;

	swift_task_come(&taskid, task->id);

	if (!service) {
		goto over;
	}

#if defined(STORE_USE_SHMQ)
	session_response_clnt(service->fd, SESSION_IO_TIMEOUT,
		"tasks(0x%20x) : %ld\n",
		g_tasks_shmqueue->shmkey,
		AO_GET(&g_tasks_shmqueue->list->nodes));

#else
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)p_swift_worker->mount;
	SNIFF_WORKER_PTHREAD    *ptr = NULL;

	for (ptr = p_sniff_worker; ptr; ) {
		flag = session_response_clnt(service->fd, SESSION_IO_TIMEOUT,
				"thread(%20ld) : %ld\n",
				ptr->tid,
				AO_GET(&ptr->thave));

		if (!flag) {
			goto over;
		}

		ptr = ptr->next;

		if (ptr == p_sniff_worker) {
			break;
		}
	}
#endif	/* if defined(STORE_USE_SHMQ) */
over:

	if (swift_task_last(&taskid, task->id)) {
		close(service->fd);
	}

	return flag;
}

static void session_dispatch_task(struct session_task *service)
{
	char type = 0;

	assert(service);
	assert(service->action);

	if ((service->action->taskmode != BIT8_TASK_TYPE_WHOLE) &&
		(service->action->taskmode != BIT8_TASK_TYPE_ALONE)) {
		type = BIT8_TASK_TYPE_ALONE;
	} else {
		type = service->action->taskmode;
	}

	struct swift_task_node task = {
		.id     = 0,
		.sfd    = 0,
		.type   = type,
		.origin = BIT8_TASK_ORIGIN_MSMQ,
		.func   = (TASK_CALLBACK)service->action->action,
		.index  = 0,
		.data   = (void *)service
	};

	if (type == BIT8_TASK_TYPE_WHOLE) {
		swift_all_task_hit(&task, false, service->fd);
	} else {
		swift_one_task_hit(&task, false, service->fd);
	}
}

void init_session_cmd()
{
	/*增加会话命令*/
	struct session_action cmd[] = {
		{ "tasks", tasks, BIT8_TASK_TYPE_WHOLE    },
	};

	session_replace_command("help", help);
	session_add_command(cmd, DIM(cmd));
	session_init_dispatch(session_dispatch_task);
}

