//
//  main.c
//  libmini
//
//  Created by 周凯 on 15/10/14.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <unistd.h>
#include <pthread.h>

#include "coro_cluster.h"
#include "libmini.h"

struct taskargs
{
	int     id;
	int     cycle;
	int     rc;
	int     exec;
};

struct startarg
{
	volatile int    counter;
	volatile int    start;
};

int normaltask(struct coro_cluster *coroptr, void *usr)
{
	struct taskargs *arg = usr;

	int i = 0;

	x_printf(W, "ID %d start ...", arg->id);

	if (arg->cycle < 1) {
		x_printf(E, "ID %d exit ...", arg->id);
		arg->exec = 1;
		return -1;
	}

	for (i = 0; i < arg->cycle; i++) {
		coro_cluster_fastswitch(coroptr);
		x_printf(I, "ID %d : %d", arg->id, i);
#if 1
		usleep(5000);
#else
		sched_yield();
#endif
	}

	x_printf(W, "ID %d end ...", arg->id);

	arg->exec = 1;
	return arg->rc;
}

void *test(void *arg)
{
	struct startarg *st = arg;

	assert(arg);

	struct coro_cluster *cluster = NULL;

	BindCPUCore(-1);

	/*同步运行*/
#ifdef __LINUX__
	futex_add_signal((int *)&st->counter, 1, 1);
	futex_cond_wait((int *)&st->start, 1, -1);
#else
	AO_INC(&st->counter);

	while (st->start != 1) {
		sched_yield();
	}
#endif
	cluster = coro_cluster_create();

	assert(cluster);

	struct taskargs task[] = {
		{ 0, 2,  0,  0 },
		{ 0, 4,  0,  0 },
		{ 0, 6,  0,  0 },
		{ 0, 3,  -1, 0 },
		{ 0, 10, 0,  0 },
		{ 0, 12, 0,  0 },
		{ 0, 14, -1, 0 },
		{ 0, 20, 0,  0 },
		{ 0, 15, 0,  0 },
		{ 0, 32, 0,  0 },
	};

	int     i = 0;
	int     tasks = (int)(sizeof(task) / sizeof(task[0]));

	for (i = 0; i < tasks; i++) {
		task[i].id = i;
		coro_cluster_add(cluster, normaltask, &task[i], 0);
	}

	bool rc = false;
	rc = coro_cluster_startup(cluster, NULL, NULL);

	if (!rc) {
		x_printf(E, "task cluster execute fail ");
	} else {
		x_printf(W, "task cluster execute success ");
	}

	coro_cluster_destroy(cluster);

	for (i = 0; rc && i < tasks; i++) {
		assert(task[i].exec);
	}

	return NULL;
}

int main()
{
	int             i = 0;
	long            ncpus = 0;
	pthread_t       *tid = NULL;

	struct startarg st = { 0, 0 };

	SignalMaskBlockAll(NULL);

	Signal(SIGUSR2, SignalNormalCB);

	ncpus = GetCPUCores();
	NewArray0(ncpus, tid);

	for (i = 0; i < ncpus; i++) {
		pthread_create(&tid[i], NULL, test, &st);
	}

#ifdef __LINUX__
	futex_cond_wait((int *)&st.counter, (int)ncpus, -1);
	futex_set_signal((int *)&st.start, 1, (int)ncpus);
#else
	while (st.counter != (int)ncpus) {
		sched_yield();
	}
	AO_SET(&st.start, 1);
#endif

	for (i = 0; i < ncpus; i++) {
		pthread_join(tid[i], NULL);
	}

	return EXIT_SUCCESS;
}

