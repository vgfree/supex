#pragma once

#include "x_utils.h"
#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "list.h"
#include "net_cache.h"
#include "mfptp_task.h"
#include "mfptp_cfg.h"
#include "supex.h"
#include "user_record_def.h"

struct mfptp_settings
{
	struct mfptp_cfg_list *conf;
};

// #define MFPTP_PUSH_SOCKET_ENDPOINT "inproc://mfptp.queue"
#define MFPTP_PUSH_SOCKET_ENDPOINT      "tcp://127.0.0.1:8123"
#define MFPTP_PUBLISH_ENDPOINT          "tcp://127.0.0.1:6666"
// #define MFPTP_PUSH_SOCKET_ENDPOINT "tcp://127.0.0.1:5559"

/*
 * libev default loop, with a accept_watcher to accept the new connect
 * and dispatch to MFPTP_WORKER_PTHREAD.
 */
typedef struct
{
	struct ev_io            msmq_watcher;	/* msmq watcher for new message */

	unsigned int            robin;
	pthread_t               thread_id;	/* unique ID of this thread */

	struct ev_loop          *loop;		/* libev loop this thread uses */
	struct ev_io            accept_watcher;	/* accept watcher for new connect */
	struct ev_timer         update_watcher;
	struct ev_signal        signal_watcher;
	struct ev_signal        log_signal_watcher;
} MASTER_PTHREAD;

/*
 * Each libev instance has a async_watcher, which other threads
 * can use to signal that they've put a new connection on its queue.
 */
typedef struct
{
	pthread_t               thread_id;	/* unique ID of this thread */
	struct queue_list       qlist;		/* queue of new request to handle */

	struct mfptp_task_node  task;
	struct supex_evuv       evuv;
	struct ev_loop          *loop;		/* libev loop this thread uses */
#ifdef USE_PIPE
	int                     pfds[2];
	struct ev_io            pipe_watcher;		/* accept watcher for new connect */
	int                     weibo_pfds[2];
	struct ev_io            weibo_pipe_watcher;	/* accept watcher for new connect */
#else
	struct ev_async         async_watcher;		/* async watcher for new connect */
#endif
	struct ev_check         check_watcher;
	struct ev_async         weibo_async_watcher;	/* async watcher for new weibo */
	void                    *zmq_socket;
	void                    *data;
	int                     index;
	int                     thave;		/*can't use unsigned int*/
	struct free_queue_list  tlist;
	struct free_queue_list  *glist;
} MFPTP_WORKER_PTHREAD;

/*******************************************/
int mfptp_mount(struct mfptp_cfg_list *conf);

int mfptp_start(void);

int mfptp_msg_pour(void *user, void *task);

