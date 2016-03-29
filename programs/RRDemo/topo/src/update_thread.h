#pragma once

#include "ev.h"
#include <stdio.h>
#include <pthread.h>

typedef struct
{
	int             index;
	pthread_t       thread_id;			/* unique ID of this thread */

	struct ev_loop  *loop;				/* libev loop this thread uses */
	struct ev_timer count_watcher;			/* */
} UPDATE_WORKER_THREAD;

void start_update_pthread(void *func, int num, void *addr, void *data);

static void *start_update(void *arg);

static void init_staworker(UPDATE_WORKER_THREAD *update_worker);

void update_cb(struct ev_loop *loop, ev_timer *w, int revents);

int update_start(int COUNTS);

