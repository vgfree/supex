/**
 * @file   rinoo_spawn_stop.c
 * @author reginaldl <reginald.l@gmail.com> - Copyright 2013
 * @date   Wed Jan  2 15:13:07 2013
 *
 * @brief  RiNOO spawn stop unit test
 *
 *
 */

#include "rinoo/rinoo.h"

#define NBSPAWNS	10

int checker[NBSPAWNS + 1];

void task(void *unused(arg))
{
	t_sched *cur;

	rinoo_log("%s start %d", __FUNCTION__, rinoo_sched_self()->id);
	/* sleep should return as soon as we get killed by the main scheduler */
	sleep(1000);
	cur = rinoo_sched_self();
	XTEST(cur != NULL);
	XTEST(cur->id >= 0 && cur->id <= NBSPAWNS);
	XTEST(checker[cur->id] == 0);
	checker[cur->id] = 1;
	rinoo_log("%s end %d", __FUNCTION__, rinoo_sched_self()->id);
}

/**
 * Wait 1 second and stop schedulers.
 * This needs to be called from the main scheduler.
 *
 * @param sched Pointer to the main scheduler
 */
void wait_and_stop(void *sched)
{
	t_sched *cur;

	cur = rinoo_sched_self();
	XTEST(cur == sched);
	XTEST(cur->id == 0);
	checker[cur->id] = 1;
	rinoo_log("wait & stop");
	rinoo_task_wait(sched, 1000);
	rinoo_log("stopping");
	rinoo_sched_stop(sched);
}

/**
 * Main function for this unit test
 *
 *
 * @return 0 if test passed
 */
int main()
{
	int i;
	t_sched *cur;
	t_sched *sched;

	memset(checker, 0, sizeof(*checker) * NBSPAWNS);
	sched = rinoo_sched();
	XTEST(sched != NULL);
	XTEST(rinoo_spawn(sched, NBSPAWNS) == 0);
	for (i = 1; i <= NBSPAWNS; i++) {
		cur = rinoo_spawn_get(sched, i);
		XTEST(cur != NULL);
		XTEST(rinoo_task_start(cur, task, NULL) == 0);
	}
	XTEST(rinoo_task_start(sched, wait_and_stop, sched) == 0);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	for (i = 0; i <= NBSPAWNS; i++) {
		XTEST(checker[i] == 1);
	}
	XPASS();
}
