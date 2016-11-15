/**
 * @file   rinoo_task_wait.c
 * @author Reginad Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Wed Feb 27 15:25:13 2013
 *
 * @brief  rinoo_task_wait unit test
 *
 *
 */

#include "rinoo/rinoo.h"

#ifdef RINOO_DEBUG
#include <valgrind/valgrind.h>
#define LATENCY			10 + RUNNING_ON_VALGRIND * 110
#else
#define LATENCY			10
#endif /* !RINOO_DEBUG */

int check_time(struct timeval *prev, uint32_t ms)
{
	uint32_t diffms;
	struct timeval cur;
	struct timeval diff;

	if (gettimeofday(&cur, NULL) != 0) {
		return -1;
	}
	timersub(&cur, prev, &diff);
	diffms = diff.tv_sec * 1000;
	diffms += diff.tv_usec / 1000;
	rinoo_log("Time diff found: %u, expected: %u - %u", diffms, ms, LATENCY);
	if (diffms > ms) {
		diffms = diffms - ms;
	} else {
		diffms = ms - diffms;
	}
	if (diffms > LATENCY) {
		return -1;
	}
	*prev = cur;
	return 0;
}

void task_func(void *sched)
{
	struct timeval prev;

	printf("%s start\n", __FUNCTION__);
	XTEST(gettimeofday(&prev, NULL) == 0);
	rinoo_task_wait(sched, 100);
	XTEST(check_time(&prev, 100) == 0);
	rinoo_task_wait(sched, 200);
	XTEST(check_time(&prev, 200) == 0);
	rinoo_task_wait(sched, 500);
	XTEST(check_time(&prev, 500) == 0);
	rinoo_task_wait(sched, 1000);
	XTEST(check_time(&prev, 1000) == 0);
	printf("%s end\n", __FUNCTION__);
}

/**
 * Main function for this unit test
 *
 *
 * @return 0 if test passed
 */
int main()
{
	t_sched *sched;

	sched = rinoo_sched();
	XTEST(sched != NULL);
	XTEST(rinoo_task_start(sched, task_func, sched) == 0);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	XPASS();
}
