#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "base/utils.h"
#include "crzpt_cfg.h"
#include "adopt_tasks/adopt_task.h"
#include "supex.h"

#define SIG_APP_ADD     36
#define SIG_APP_SET     37
#define SIG_APP_DEL     38

enum
{
	CRZPT_MSMQ_TYPE_PUSH = 1,	// for lua VM
	CRZPT_MSMQ_TYPE_PULL,		// for lua VM

	CRZPT_MSMQ_TYPE_INSERT,		// for task list
	CRZPT_MSMQ_TYPE_UPDATE,		// for task list
	CRZPT_MSMQ_TYPE_SELECT,		// for task list
};

struct crzpt_settings
{
	struct crzpt_cfg_list *conf;
};

typedef struct
{
	pthread_t               thread_id;	/* unique ID of this thread */

	struct ev_loop          *loop;		/* libev loop this thread uses */
	struct ev_idle          idle_watcher;
#ifdef CRZPT_OPEN_MSMQ
	struct ev_io            msmq_watcher;	/* msmq watcher for new message */
#else
  #if 0
	struct ev_signal        signal_quit_watcher;
	struct ev_signal        signal_add_watcher;
	struct ev_signal        signal_set_watcher;
	struct ev_signal        signal_del_watcher;
  #endif
#endif
} CRZPT_MASTER_PTHREAD;

typedef struct
{
	void                    *data;
	// int batch;//FIXME
	int                     index;
	pthread_t               thread_id;	/* unique ID of this thread */
	struct free_queue_list  tlist;
} CRZPT_WORKER_PTHREAD;

int crzpt_mount(struct crzpt_cfg_list *conf);

int crzpt_start(void);

