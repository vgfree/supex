/**
 * @file   rinoo_spawn.c
 * @author reginaldl <reginald.l@gmail.com> - Copyright 2013
 * @date   Wed Jan  2 15:13:07 2013
 *
 * @brief  RiNOO spawn unit test
 *
 *
 */

#include "rinoo/rinoo.h"

#define NBSPAWNS	10

int checker[NBSPAWNS + 1];

void task(void *unused(arg))
{
	t_sched *cur;

	printf("%s start\n", __FUNCTION__);
	cur = rinoo_sched_self();
	XTEST(cur != NULL);
	XTEST(cur->id >= 0 && cur->id <= NBSPAWNS);
	XTEST(checker[cur->id] == 0);
	checker[cur->id] = 1;
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
	int i;
	t_sched *cur;
	t_sched *sched;

	memset(checker, 0, sizeof(*checker) * NBSPAWNS);
	sched = rinoo_sched();
	XTEST(sched != NULL);
	XTEST(rinoo_spawn(sched, NBSPAWNS) == 0);
	for (i = 0; i <= NBSPAWNS; i++) {
		cur = rinoo_spawn_get(sched, i);
		XTEST(cur != NULL);
		if (i == 0) {
			XTEST(cur == sched);
		}
		XTEST(rinoo_task_start(cur, task, sched) == 0);
	}
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	for (i = 0; i <= NBSPAWNS; i++) {
		XTEST(checker[i] == 1);
	}
	XPASS();
}
