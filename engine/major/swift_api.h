#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "list.h"
#include "net_cache.h"
#include "utils.h"
#include "scco.h"
#include "swift_task.h"
#include "swift_cfg.h"
#include "supex.h"

struct swift_settings
{
	struct swift_cfg_list *conf;

	union
	{
		/* http */
		struct api_list         apis[MAX_API_COUNTS + 1];	/*all api counts can't big then MAX_API_COUNTS*/
		/* redis */
		struct cmd_list         cmds[MAX_CMD_COUNTS + 1];	/*all cmd counts can't big then MAX_API_COUNTS*/
#ifdef _mttptest
		struct mttp_list        mcbs[MAX_MCB_COUNTS + 1];	/*all mcb counts can't big then MAX_API_COUNTS*/
#endif
	};
};

/*
 * libev default loop, with a accept_watcher to accept the new connect
 * and dispatch to SWIFT_WORKER_PTHREAD.
 */
typedef struct
{
	struct ev_io            msmq_watcher;		/* msmq watcher for new message */

	unsigned int            robin;
	pthread_t               thread_id;		/* unique ID of this thread */

	struct ev_loop          *loop;			/* libev loop this thread uses */
	struct ev_io            accept_watcher;		/* accept watcher for new connect */
	struct ev_io            session_watcher;
	struct ev_timer         update_watcher;
	struct ev_signal        sigquit_watcher;
	struct ev_signal        sigint_watcher;
	struct ev_signal        sigpipe_watcher;
	struct ev_stat          file_watcher;
} SWIFT_MASTER_PTHREAD;

/*
 * Each libev instance has a async_watcher, which other threads
 * can use to signal that they've put a new connection on its queue.
 */
typedef struct
{
	int                     index;

#ifdef OPEN_EQUAL
	int                     work_num;	/* pthread work number */
#endif

	unsigned int            depth;
	pthread_t               thread_id;	/* unique ID of this thread */
	union virtual_system    VMS;
	struct swift_task_node  task;
	struct queue_list       qlist;		/* queue of new request to handle */
	struct supex_task_list  tlist;		/* use prepare_watcher to do manage task */
	void                    *mount;

	struct ev_loop          *loop;			/* libev loop this thread uses */
#ifdef USE_PIPE
	int                     pfds[2];
	struct ev_io            pipe_watcher;		/* 侦听管道中的连接 */
#else
	struct ev_async         async_watcher;		/* async watcher for new connect */
#endif
	struct ev_prepare       prepare_watcher;	/* 每次事件循环开始弹出所有任务并运行*/
	struct ev_idle          idle_watcher;
	struct ev_check         check_watcher;		/* 每次事件循环结束，检查队列和管道是否有就绪的连接*/
} SWIFT_WORKER_PTHREAD;

extern SWIFT_WORKER_PTHREAD     *g_swift_worker_pthread;
extern int                      SWIFT_WORKER_COUNTS;
/*******************************************/
int swift_mount(struct swift_cfg_list *conf);

int swift_start(void);

void swift_suspend_thread(struct ThreadSuspend *cond);

typedef int (*SWIFT_VMS_FCB)(lua_State **L, int last, struct swift_task_node *task);

int swift_for_alone_vm(void *W, SWIFT_VMS_FCB vms_fcb);

