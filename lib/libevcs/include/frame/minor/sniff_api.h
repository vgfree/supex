#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "utils.h"
#include "engine/supex.h"
#include "sniff_cfg.h"
#include "sniff_task.h"
#include "engine/base/free_queue.h"
#include "engine/thread_pool_loop/tlpool.h"

struct sniff_settings
{
	struct sniff_cfg_list *conf;
};

extern int G_SNIFF_WORKER_COUNTS;
/* -------------                             */

int sniff_mount(struct sniff_cfg_list *conf);

/**< 创造的批次*/
tlpool_t *sniff_start(void *data);

void sniff_all_task_hit(tlpool_t *tlpool, struct sniff_task_node *task);

void sniff_one_task_hit(tlpool_t *tlpool, struct sniff_task_node *task);

bool sniff_suspend_thread(tlpool_t *tlpool, struct ThreadSuspend *cond);

