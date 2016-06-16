/*
 *  update_thread.c
 *
 *  Created on: Feb 25, 2016
 *  Author: shu
 * */

#include "update_thread.h"
#include "binheap.h"
#include "calculate.h"
#include "utils.h"

#define CYCLE_TIME 1

UPDATE_WORKER_THREAD *g_update_thread = NULL;

void start_update_pthread(void *func, int num, void *addr, void *data)
{
	pthread_attr_t  attr;
	pthread_t       thread;

	struct safe_init_base base = {};

	pthread_mutex_init(&base.lock, NULL);
	pthread_cond_init(&base.cond, NULL);

	struct safe_init_step slot[num];

	int i = 0;

	for (i = 0; i < num; i++) {
		slot[i].base = &base;
		slot[i].step = i;
		slot[i].addr = addr;
		slot[i].data = data;

		pthread_attr_init(&attr);

		/* create thread */
		if (0 != pthread_create(&thread, &attr, func, (void *)&slot[i])) {
			x_perror("Can't create thread");
			exit(EXIT_FAILURE);
		}
	}

	/* Wait for all the threads to set themselves up before returning. */
	pthread_mutex_lock(&base.lock);

	while (base.count < num) {
		struct timespec tm = {};

		/*
		 * 一段时间唤醒一次检查所有线程是否完成启动
		 */
		TM_FillAbsolute(&tm, 500);

		pthread_cond_timedwait(&base.cond, &base.lock, &tm);
	}

	pthread_mutex_unlock(&base.lock);

	sleep(1);	/*just to delay*/
}

static void *start_update(void *arg)
{
	UPDATE_WORKER_THREAD    *update_worker = NULL;
	struct safe_init_step   *info = arg;

	SAFE_PTHREAD_INIT_COME(info);

	int idx = info->step;

	update_worker = &g_update_thread[idx];

	update_worker->index = idx;
	update_worker->thread_id = pthread_self();

	init_staworker(update_worker);

	SAFE_PTHREAD_INIT_OVER(info);
	/*start loop*/
	ev_loop(update_worker->loop, 0);
	/*exit*/
	ev_loop_destroy(update_worker->loop);
	return NULL;
}

static void init_staworker(UPDATE_WORKER_THREAD *update_worker)
{
	update_worker->loop = ev_loop_new(EVBACKEND_EPOLL | EVFLAG_NOENV);
	update_worker->count_watcher.data = update_worker;
	ev_timer_init(&(update_worker->count_watcher), update_cb, CYCLE_TIME * 20, CYCLE_TIME);
	ev_timer_start(update_worker->loop, &(update_worker->count_watcher));
}

void update_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
	printf("update ..\n");
	UPDATE_WORKER_THREAD *p_worker = w->data;
	update_call();
	p_worker->index++;
}

int update_start(int COUNTS)
{
	NewArray0(COUNTS, g_update_thread);
	start_update_pthread((void *)start_update, COUNTS, NULL, NULL);
	return 0;
}

