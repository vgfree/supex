/**
 * @file   rinoo_task_pause.c
 * @author reginaldl <reginald.l@gmail.com> - Copyright 2013
 * @date   Wed Feb 27 15:25:13 2013
 *
 * @brief  rinoo_task_pause unit test
 *
 *
 */

#include "rinoo/rinoo.h"

int check = 0;

void task1(void *sched)
{
	int i;

	printf("%s start\n", __FUNCTION__);
	for (i = 0; i < 10; i++) {
		check++;
		printf("%s - %d\n", __FUNCTION__, check);
		XTEST(check == 1);
		rinoo_task_pause(sched);
	}
	printf("%s end\n", __FUNCTION__);
}

void task2(void *sched)
{
	int i;

	printf("%s start\n", __FUNCTION__);
	for (i = 0; i < 10; i++) {
		check++;
		printf("%s - %d\n", __FUNCTION__, check);
		XTEST(check == 2);
		rinoo_task_pause(sched);
	}
	printf("%s end\n", __FUNCTION__);
}

void task3(void *sched)
{
	int i;

	printf("%s start\n", __FUNCTION__);
	for (i = 0; i < 10; i++) {
		check++;
		printf("%s - %d\n", __FUNCTION__, check);
		XTEST(check == 3);
		check = 0;
		rinoo_task_pause(sched);
	}
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
	XTEST(rinoo_task_start(sched, task2, sched) == 0);
	XTEST(rinoo_task_start(sched, task3, sched) == 0);
	rinoo_sched_loop(sched);
	rinoo_sched_destroy(sched);
	XPASS();
}
