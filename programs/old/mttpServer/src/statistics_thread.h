#pragma one

#include "ev.h"
#include "rr_cfg.h"
#include <stdio.h>
#include <pthread.h>

typedef struct
{
	int             index;
	pthread_t       thread_id;			/* unique ID of this thread */

	struct ev_loop  *loop;				/* libev loop this thread uses */
	struct ev_timer count_watcher;			/* */
} STATISTICS_WORKER_THREAD;

void start_statistics_pthread(void *func, int num, void *addr, void *data);

static void *start_statistics(void *arg);

static void init_staworker(STATISTICS_WORKER_THREAD *statistics_worker);

void statistics_cb(struct ev_loop *loop, ev_timer *w, int revents);

int statistics_start(int COUNTS);

