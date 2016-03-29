//
//  test_libev2.c
//  libmini
//
//  Created by 周凯 on 15/12/1.
//  Copyright © 2015年 zk. All rights reserved.
//
#include <ev.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void tocall(struct ev_loop *loop, struct ev_timer *timer, int event)
{
	fprintf(stdout, "timed out ...\n");
	ev_timer_stop(loop, timer);
}

int main()
{
	struct ev_loop *loop = ev_default_loop(0);

	struct ev_timer tm;

	ev_timer_init(&tm, tocall, 4., .0);
	ev_timer_start(loop, &tm);

	fprintf(stdout, "first loop\n");
	ev_loop(loop, EVRUN_ONCE | EVRUN_NOWAIT);

	sleep(5);

	fprintf(stdout, "second loop\n");
	ev_loop(loop, EVRUN_ONCE);

	ev_loop_destroy(loop);

	return EXIT_SUCCESS;
}

