/**
 * @file   rinoo_scheduler_stop.c
 * @author reginaldl <reginald.l@gmail.com> - Copyright 2013
 * @date   Wed Jan  2 15:13:07 2013
 *
 * @brief  RiNOO Scheduler stop unit test
 *
 *
 */

#include "rinoo/rinoo.h"

int checker = 0;

void task(void *unused(arg))
{
	printf("%s start\n", __FUNCTION__);
	XTEST(checker == 0);
	checker = 1;
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
	XTEST(rinoo_task_start(sched, task, sched) == 0);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	XTEST(checker == 1);
	XPASS();
}
