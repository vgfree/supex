/**
 * @file   rinoo_task.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Fri Jan 13 18:07:14 2012
 *
 * @brief  rinoo_task unit test
 *
 *
 */

#include "rinoo/rinoo.h"

int checker = 0;

void task3(void *arg)
{
	t_sched *sched = arg;
	printf("%s start\n", __FUNCTION__);
	XTEST(checker == 2);
	checker = 3;
	rinoo_sched_stop(sched);
	printf("%s end\n", __FUNCTION__);
}

void task2(void *arg)
{
	t_sched *sched = arg;

	printf("%s start\n", __FUNCTION__);
	XTEST(checker == 1);
	rinoo_task_start(sched, task3, sched);
	XTEST(checker == 1);
	checker = 2;
	printf("%s end\n", __FUNCTION__);
}

void task1(void *arg)
{
	t_sched *sched = arg;

	printf("%s start\n", __FUNCTION__);
	XTEST(checker == 0);
	rinoo_task_start(sched, task2, sched);
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
	XTEST(rinoo_task_start(sched, task1, sched) == 0);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	XTEST(checker == 3);
	XPASS();
}
