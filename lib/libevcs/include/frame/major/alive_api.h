#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "cache/cache.h"
#include "base/utils.h"
#include "scco.h"
#include "adopt_tasks/adopt_task.h"
#include "alive_cfg.h"
#include "async_comm.h"
#include "supex.h"
#include "../listen_pthread.h"
#include "base/free_queue.h"

struct alive_settings
{
	struct alive_cfg_list *conf;

	union
	{
		/* http */
		struct api_list apis[MAX_API_COUNTS + 1];		/*all api counts can't big then MAX_API_COUNTS*/
		/* redis */
		struct cmd_list cmds[MAX_CMD_COUNTS + 1];		/*all cmd counts can't big then MAX_API_COUNTS*/
		/* mttp */
		struct mcb_list mcbs[MAX_MCB_COUNTS + 1];		/*all mcb counts can't big then MAX_API_COUNTS*/
	};
};

typedef struct
{
	struct ev_loop          *loop;		/**< 该句柄的事件侦听器 libev loop this thread uses */
#ifdef OPEN_EVCORO
	struct supex_evcoro     *p_evcs;
#endif
	int                     ptype;
	struct accept_module    mdl_accept;

	void                    *mount;
	void                    *data;
	struct free_queue_list  tlist;		/**< 任务队列*/
} ALIVE_WORKER_PTHREAD;

/*******************************************/
int alive_mount(struct alive_cfg_list *conf);

int alive_start(void);

void alive_suspend_thread(struct ThreadSuspend *cond);

void alive_send_data(bool self, uint64_t cid, int sfd, char *data, long size);

void alive_close_conn(bool self, uint64_t cid, int sfd);
