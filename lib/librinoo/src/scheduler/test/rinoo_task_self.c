/**
 * @file   rinoo_task_self.c
 * @author Reginald Lips <reginald.l@gmail.com> - Copyright 2013
 * @date   Tue May  7 08:43:33 2013
 *
 * @brief  rinoo_task_self unit test
 *
 *
 */

#include "rinoo/rinoo.h"

t_sched *sched;

void task3(void *unused(arg))
{
	printf("%s start\n", __FUNCTION__);
	XTEST(rinoo_task_self() == sched->driver.current);
	printf("%s end\n", __FUNCTION__);
}

void task2(void *unused(arg))
{
	printf("%s start\n", __FUNCTION__);
	XTEST(rinoo_task_self() == sched->driver.current);
	rinoo_task_run(sched, task3, sched);
	XTEST(rinoo_task_self() == sched->driver.current);
	printf("%s end\n", __FUNCTION__);
}

void task1(void *unused(arg))
{
	printf("%s start\n", __FUNCTION__);
	XTEST(rinoo_task_self() == sched->driver.current);
	rinoo_task_run(sched, task2, sched);
	XTEST(rinoo_task_self() == sched->driver.current);
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
	XTEST(rinoo_task_self() == NULL);
	sched = rinoo_sched();
	XTEST(sched != NULL);
	XTEST(rinoo_task_run(sched, task1, sched) == 0);
	rinoo_sched_destroy(sched);
	XPASS();
}
