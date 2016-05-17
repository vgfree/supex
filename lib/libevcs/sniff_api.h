#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "utils.h"
#include "supex.h"
#include "sniff_cfg.h"
#include "sniff_task.h"

struct sniff_settings
{
	struct sniff_cfg_list *conf;
};

typedef struct sniff_worker_pthread
{
#if defined(OPEN_SCCO) || defined(OPEN_EVCORO)
#endif
	void                            *user;			/**< */

	void                            *data;			/**< 所属swift的指针*/
	int                             batch;			/**< 创造的批次*/
	int                             genus;			/*延迟路由、非延迟路由的*/

	int                             index;
	pthread_t                      	pid;		/* unique ID of this thread */
	long                            tid;
	AO_T                            thave;			/*can't use unsigned int*/
	struct supex_task_list          tlist;
	struct supex_task_list          *glist;
	struct sniff_worker_pthread     *next;
} SNIFF_WORKER_PTHREAD;

extern int G_SNIFF_WORKER_COUNTS;
/* -------------                             */

int sniff_mount(struct sniff_cfg_list *conf);

SNIFF_WORKER_PTHREAD *sniff_start(void *data, int batch, int genus);


void sniff_all_task_hit(SNIFF_WORKER_PTHREAD *head, struct sniff_task_node *task);

void sniff_one_task_hit(SNIFF_WORKER_PTHREAD *head, struct sniff_task_node *task);

bool sniff_suspend_thread(SNIFF_WORKER_PTHREAD *sniff_worker, struct ThreadSuspend *cond);

