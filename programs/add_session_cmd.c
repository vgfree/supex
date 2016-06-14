//
//  addition_session_command.c
//  supex
//
//  Created by 周凯 on 15/8/5.
//  Copyright (c) 2015年 zk. All rights reserved.
//

#include "add_session_cmd.h"
#include "minor/sniff_api.h"
#include "major/swift_api.h"
#include "apply_def.h"
#include "major/swift_evcb.h"

ShmQueueT g_tasks_shmqueue = NULL;

static bool help(void *user, void *data)
{
	bool flag = false;

	static char helpinfo[] = "[help] to list what has been supported commands.\n"
#if defined(USE_MEMHOOK)
		"[memblocks] to check how much blocks that were allocated in heap.\n"
		"[memdetails] to print information about memory that were allocated in heap.\n"
#endif
		"[tasks] to check how much tasks that belong to parters.\n"
		"[netpkgstat] to check how much network package that have been recieved.\n";

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
	struct adopt_task_node  *task = NULL;
	struct session_task     *service = NULL;
	SWIFT_WORKER_PTHREAD    *p_swift_worker = NULL;

	assert(user);
	assert(data);

	task = (struct adopt_task_node *)data;
	p_swift_worker = (SWIFT_WORKER_PTHREAD *)user;
	service = (struct session_task *)task->data;

	//swift_task_come(&taskid, task->id);

	if (!service) {
		goto over;
	}
#if 0
#if defined(STORE_USE_SHMQ)
	session_response_clnt(service->fd, SESSION_IO_TIMEOUT,
		"tasks(0x%20x) : %ld\n",
		g_tasks_shmqueue->shmkey,
		AO_GET(&g_tasks_shmqueue->list->nodes));

#else
	flag = session_response_clnt(service->fd, SESSION_IO_TIMEOUT,
			"thread(%20ld) : %ld\n",
			ptr->tid,
			AO_GET(&ptr->thave));

	if (!flag) {
		goto over;
	}

#endif	/* if defined(STORE_USE_SHMQ) */
#endif
over:

	//if (swift_task_last(&taskid, task->id)) {
		close(service->fd);
	//}

	return flag;
}

static bool netpkgstat(void *user, void *data)
{
	bool flag = false;

	struct session_task *service = NULL;

	assert(data);
	service = (struct session_task *)data;

	if (!service) {
		return false;
	}

	flag = session_response_clnt(service->fd,
			SESSION_IO_TIMEOUT,
			"%15s | %15s | %20s | %20s \n",
			"ip address", "package counts",
			"connect time", "recent time");

	netpkg_printstat(g_netpkg_stat, service->fd, NULL);

	/* ---- */

	return true;
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

	struct adopt_task_node task = {
		.id     = 0,
		.sfd    = 0,
		.type   = type,
		.origin = BIT8_TASK_ORIGIN_MSMQ,
		.func   = (TASK_VMS_FCB)service->action->action,
		.index  = 0,
		.data   = (void *)service
	};

	if (type == BIT8_TASK_TYPE_WHOLE) {
		swift_all_task_hit(&task, false, service->fd);
	} else {
		swift_one_task_hit(&task, false, service->fd, 0);
	}
}

void init_session_cmd()
{
	/*
	 * 会话扩展命令
	 */
	struct session_action cmd[] = {
		{ "tasks",      tasks,      BIT8_TASK_TYPE_WHOLE    },
		{ "netpkgstat", netpkgstat, 0                       },
	};

	/*初始化会话命令相关*/
	session_replace_command("help", help);
	session_add_command(cmd, DIM(cmd));
	session_init_dispatch(session_dispatch_task);
}

