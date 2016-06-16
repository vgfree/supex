//
//  test.c
//  libmini
//
//  Created by 刘绍宸 on 16/02/22.
//  Copyright © 2016年 lsc. All rights reserved.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "evcoro_scheduler.h"

#define RandInt(low, high) \
	((long)((((double)random()) / ((double)RAND_MAX + 1)) * (high - low + 1)) + low)

/*
 * 随机获取小数 x ： low <= x < high
 */
#define RandReal(low, high) \
	((((double)random()) / ((double)RAND_MAX + 1)) * (high - low) + low)

/*
 * 测试一次概率为 p(分数) 的事件
 */
#define RandChance(p) (RandReal(0, 1) < (double)(p) ? true : false)
static int task_id = 0;

struct task1
{
	int     id;
	int     cycle;
	int     rc;
	bool    m;
};

struct task2
{
	int     id;
	char    name[24];
};

struct task3
{
	int     id;
	char    ip[24];
};

void work1(struct evcoro_scheduler *scheduler, void *usr)
{
	struct task1 *arg = usr;

	printf("working task 1. id=%d.\n", arg->id);
	evcoro_fastswitch(scheduler);
	printf("working task 1. id=%d End.\n", arg->id);
}

void work2(struct evcoro_scheduler *scheduler, void *usr)
{
	struct task2 *arg = usr;

	evcoro_fastswitch(scheduler);
	printf("working task 2. id=%d.\n", arg->id);
	sleep(1);
	printf("working task 2. id=%d End..\n", arg->id);
}

void work3(struct evcoro_scheduler *scheduler, void *usr)
{
	printf("working task 3. id=3.\n");
	struct task3 *arg = usr;
	evcoro_fastswitch(scheduler);
	printf("working task 3. id=%d End...\n", arg->id);
}

// void work(struct evcoro_scheduler *scheduler, void *usr)
// {
//	struct task     *arg = usr;
//	int             i = 0;
//
//	printf("\x1B[1;33m" "ID %d start ..." "\x1B[m" "\n", arg->id);
//
//	if (arg->cycle < 1) {
//		printf("\x1B[1;31m" "ID %d exit ..." "\x1B[m" "\n", arg->id);
//		return;
//	}
//
//	for (i = 0; i < arg->cycle; i++) {
//		evcoro_fastswitch(scheduler);
//		printf("\x1B[1;32m" "ID %d : %d" "\x1B[m" "\n", arg->id, i + 1);
//	}
//
//	printf("\x1B[1;33m" "ID %d end ..." "\x1B[m" "\n", arg->id);
//
//	free(usr);
// }

void idle(struct evcoro_scheduler *scheduler, void *usr)
{
	struct timeval tv = {};

	gettimeofday(&tv, NULL);

	printf("\x1B[1;31m" "<<< %ld.%06d : IDLE, tasks [%d] >>>" "\x1B[m" "\n",
		tv.tv_sec, tv.tv_usec, evcoro_workings(scheduler));

	if (unlikely(evcoro_workings(scheduler) < 1)) {
		evcoro_stop(scheduler);
	}
}

int main(int argc, char **argv)
{
	struct evcoro_scheduler *scheduler = NULL;

	scheduler = evcoro_create(-1);
	assert(scheduler);

	struct task1 *t1 = calloc(1, sizeof(*t1));
	assert(t1);
	t1->id = 1;
	bool flag1 = evcoro_push(scheduler, work1, t1, 0);
	assert(flag1);

	struct task2 *t2 = calloc(1, sizeof(*t2));
	assert(t2);
	t2->id = 2;
	bool flag2 = evcoro_push(scheduler, work2, t2, 0);
	assert(flag2);

	struct task3 *t3 = calloc(1, sizeof(*t3));
	assert(t3);
	t3->id = 3;
	bool flag3 = evcoro_push(scheduler, work3, t3, 0);
	assert(flag3);

	evcoro_loop(scheduler, idle, NULL);

	evcoro_destroy(scheduler, free);

	return EXIT_SUCCESS;
}

// The execute result like down.
//
// [lba@node8 libevcoro_test]$ ./bin/multiworks_test
// working task 1. id=1.
// working task 3. id=3.
// <<< 1456124961.931415 : IDLE, tasks [3] >>>
// working task 1. id=1 End.
// working task 2. id=2.
// working task 2. id=2 End..
// working task 3. id=3 End...
// <<< 1456124962.931629 : IDLE, tasks [0] >>>

