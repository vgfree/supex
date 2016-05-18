#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "../engine/cache/cache.h"
#include "base/utils.h"
#include "../lib/libscco/scco.h"
#include "adopt_tasks/adopt_task.h"
#include "swift_cfg.h"
#include "async_comm.h"
#include "supex.h"
#include "../listen_pthread.h"
#include "../hander_pthread.h"

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
#define SWIFT_MASTER_PTHREAD	LISTEN_PTHREAD

/*
 * Each libev instance has a async_watcher, which other threads
 * can use to signal that they've put a new connection on its queue.
 */

#define SWIFT_WORKER_PTHREAD HANDER_PTHREAD

extern SWIFT_WORKER_PTHREAD     *g_swift_worker_pthread;
extern int                      SWIFT_WORKER_COUNTS;
/*******************************************/
int swift_mount(struct swift_cfg_list *conf);

int swift_start(void);

void swift_suspend_thread(struct ThreadSuspend *cond);

void swift_section_send_start(char origin, int sfd);
