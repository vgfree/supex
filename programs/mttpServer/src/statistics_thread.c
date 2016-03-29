#include "utils.h"
#include "mttp_common.h"
#include "statistics_thread.h"
#include "count.h"

extern struct rr_cfg_file       g_rr_cfg_file;
extern kv_handler_t             *count_handler;

STATISTICS_WORKER_THREAD *g_statistics_thread = NULL;

void start_statistics_pthread(void *func, int num, void *addr, void *data)
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

static void *start_statistics(void *arg)
{
	STATISTICS_WORKER_THREAD        *statistics_worker = NULL;
	struct safe_init_step           *info = arg;

	SAFE_PTHREAD_INIT_COME(info);

	int idx = info->step;

	statistics_worker = &g_statistics_thread[idx];

	statistics_worker->index = idx;
	statistics_worker->thread_id = pthread_self();

	init_staworker(statistics_worker);

	SAFE_PTHREAD_INIT_OVER(info);
	/*start loop*/
	ev_loop(statistics_worker->loop, 0);
	/*exit*/
	ev_loop_destroy(statistics_worker->loop);
	return NULL;
}

static void init_staworker(STATISTICS_WORKER_THREAD *statistics_worker)
{
	statistics_worker->loop = ev_loop_new(EVBACKEND_EPOLL | EVFLAG_NOENV);
	statistics_worker->count_watcher.data = statistics_worker;
	ev_timer_init(&(statistics_worker->count_watcher), statistics_cb, g_rr_cfg_file.synctime, g_rr_cfg_file.synctime);
	ev_timer_start(statistics_worker->loop, &(statistics_worker->count_watcher));
}

void statistics_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
	x_printf(D, "time out..\n");
	STATISTICS_WORKER_THREAD        *p_worker = w->data;
	int                             disk = data_dump(count_handler, &g_rr_cfg_file.sqlconf);

	if (disk < 0) {
		x_printf(E, "index:%d MYSQL dump failed .\n", p_worker->index);
		p_worker->index++;

		if (p_worker->index == 3) {
			x_printf(E, "index:%d MYSQL dump failed!!! exit.\n", p_worker->index);
			exit(-1);
		}
	} else {
		p_worker->index = 0;
	}
}

int statistics_start(int COUNTS)
{
	NewArray0(COUNTS, g_statistics_thread);
	start_statistics_pthread((void *)start_statistics, COUNTS, NULL, NULL);
	return GV_OK;
}

