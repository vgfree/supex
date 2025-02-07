#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "engine/cache/cache.h"
#include "engine/base/utils.h"
#include "engine/proto_comm.h"
#include "scco.h"
#include "engine/adopt_tasks/adopt_task.h"
#include "smart_cfg.h"
#include "engine/supex.h"
#include "../listen_pthread.h"
#include "../hander_pthread.h"
#include "engine/thread_pool_loop/tlpool.h"

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

#define SMART_MASTER_PTHREAD LISTEN_PTHREAD

typedef struct
{
#ifdef OPEN_EVCORO
#endif

	void            *data;
	int             batch;		/**<*/
	int             index;		/**< 所属线程池的下标*/
	pthread_t       pid;		/* unique ID of this thread */
	long            tid;
} SMART_WORKER_PTHREAD;

/*
 * Each libev instance has a async_watcher, which other threads
 * can use to signal that they've put a new connection on its queue.
 */
#define SMART_HANDER_PTHREAD HANDER_PTHREAD

extern tlpool_t *g_smart_worker_tlpool;
/*******************************************/
int smart_mount(struct smart_cfg_list *conf);

int smart_start(void);

void smart_section_send_start(char origin, int sfd);

