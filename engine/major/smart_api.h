#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "list.h"
#include "net_cache.h"
#include "utils.h"
#include "scco.h"
#include "smart_task.h"
#include "smart_cfg.h"
#include "supex.h"

enum
{
	RECV_TASK_TYPE = 0,
	SEND_TASK_TYPE = 1,
};

struct smart_settings
{
	struct smart_cfg_list *conf;

	union
	{
		/* USE_HTTP_PROTOCOL */
		struct api_list apis[MAX_API_COUNTS + 1];	/*all api counts can't big then MAX_API_COUNTS*/
		/* USE_REDIS_PROTOCOL */
		struct cmd_list cmds[MAX_CMD_COUNTS + 1];	/*all cmd counts can't big then MAX_API_COUNTS*/
	};
};

/*
 * libev default loop, with a accept_watcher to accept the new connect
 * and dispatch to SMART_HANDER_PTHREAD.
 */
typedef struct
{
	struct ev_io            msmq_watcher;	/* msmq watcher for new message */

	unsigned int            robin;		/**< 选取接收线程的下标HIT*/
	unsigned int            robout;		/**< 选取发送线程的下标HIT*/

	pthread_t               thread_id;	/* unique ID of this thread */
	long                    tid;

	struct ev_loop          *loop;			/* libev loop this thread uses */
	struct ev_io            accept_watcher;		/* accept watcher for new connect */
	struct ev_io            session_watcher;	/* accept command from fifo*/
	struct ev_timer         update_watcher;
	struct ev_periodic      monitor_watcher;	// 监控事件
	struct ev_signal        sigquit_watcher;
	struct ev_signal        sigpipe_watcher;	//
	struct ev_idle          idle_watcher;
} SMART_MASTER_PTHREAD;

typedef struct
{
#ifdef OPEN_SCCO
	struct  supex_scco      scco;
	int                     airing;		/*BIT8_TASK_TYPE_WHOLE task count*/
#endif
#ifdef OPEN_LINE
	struct supex_line       line;		/**< 任务线性处理模型 */
#endif
#ifdef OPEN_EVUV
	struct supex_evuv       evuv;
#endif
#ifdef OPEN_EVCORO
	struct supex_evcoro     evcoro;
#endif

	void                    *data;
	int                     batch;		/**<*/
	int                     index;		/**< 所属线程池的下标*/
	pthread_t               thread_id;	/* unique ID of this thread */
	long                    tid;
	struct supex_task_list  tlist;		/**< 任务队列*/
} SMART_WORKER_PTHREAD;

/*
 * Each libev instance has a async_watcher, which other threads
 * can use to signal that they've put a new connection on its queue.
 */
typedef struct
{
	int                     index;		/**< 所属接收线程池下标*/
	unsigned int            robin;		/**< 选取工作线程的下标HIT*/
	pthread_t               thread_id;	/**< 所属线程 unique ID of this thread */
	long                    tid;
#ifdef OPEN_EQUAL
	int                     work_num;	/* 任务数量 */
#endif

	struct ev_loop          *loop;		/**< 该句柄的事件侦听器 libev loop this thread uses */
	struct queue_list       recv_qlist;	/**< queue of new request to handle */
	struct queue_list       send_qlist;	/**< queue of new request to handle */
#ifdef USE_PIPE
	int                     recv_pfds[2];
	struct ev_io            recv_pipe_watcher;	/**< 读写watcher accept watcher for new connect */

	int                     send_pfds[2];
	struct ev_io            send_pipe_watcher;
#else
	struct ev_async         async_recv_watcher;	/**< 异步watcher async watcher for new connect */
	struct ev_async         async_send_watcher;	/**< 异步watcher async watcher for new connect */
#endif
	// struct ev_prepare prepare_watcher;
	struct ev_check         check_watcher;	/**< 事件检查watcher 读写事件回调时发生*/
	struct ev_idle          idle_watcher;
} SMART_HANDER_PTHREAD;

/*******************************************/
int smart_mount(struct smart_cfg_list *conf);

int smart_start(void);

#if defined(OPEN_SCCO) || defined(OPEN_EVCORO)
typedef int (*SMART_VMS_FCB)(lua_State **L, int last, struct smart_task_node *task, long S);

int smart_for_alone_vm(void *user, void *task, int step, SMART_VMS_FCB vms_fcb);

int smart_for_batch_vm(void *user, void *task, int step, SMART_VMS_FCB vms_fcb);
#endif
#if defined(OPEN_LINE) || defined(OPEN_EVUV)
typedef int (*SMART_VMS_FCB)(lua_State **L, int last, struct smart_task_node *task);

int smart_for_alone_vm(void *user, void *task, SMART_VMS_FCB vms_fcb);
#endif

