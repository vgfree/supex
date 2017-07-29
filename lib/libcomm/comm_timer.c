/********************************************************
 * Filename: comm_timer.c
 * Author: baoxue
 * Desprition:
 * Date: 2017-07-24
 ********************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/timerfd.h>
#include <unistd.h>
#include <fcntl.h>

#include "comm_utils.h"
#include "comm_timer.h"

int commtimer_init(struct comm_timer *timer)
{
	int tmfd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (tmfd < 0) {
		loger("timerfd_create error, Error:[%d:%s]", errno, strerror(errno));
		timer->tmfd = -1;
		return -1;
	}
	/* 设置为非阻塞模式 */
	if (fcntl(tmfd, F_SETFL, O_NONBLOCK) == -1) {
		close(tmfd);
		timer->tmfd = -1;
		return -1;
	}
	timer->tmfd = tmfd;
	timer->active = false;
	return 0;
}

int commtimer_wait(struct comm_timer *timer, long delay)
{
	if (delay == 0) {
		return -1;
	}
	struct itimerspec new_value;

	new_value.it_value.tv_sec = delay / 1000;
	new_value.it_value.tv_nsec = (delay % 1000) * 1000 * 1000;
	new_value.it_interval.tv_sec = 0;
	new_value.it_interval.tv_nsec = 0;

	int ret = timerfd_settime(timer->tmfd, 0, &new_value, NULL);
	if (ret < 0) {
		loger("timerfd_settime error, Error:[%d:%s]", errno, strerror(errno));
		return -1;
	}
	timer->active = true;
	return 0;
}

int commtimer_stop(struct comm_timer *timer)
{
	struct itimerspec new_value = {0};
	int ret = timerfd_settime(timer->tmfd, 0, &new_value, NULL);
	if (ret < 0) {
		loger("timerfd_settime error, Error:[%d:%s]", errno, strerror(errno));
		return -1;
	}
	timer->active = false;
	return 0;
}

uint64_t commtimer_grab(struct comm_timer *timer)
{
	uint64_t exp = 0;

	read(timer->tmfd, &exp, sizeof(uint64_t)); 
	timer->active = false;
	return exp;
}

int commtimer_free(struct comm_timer *timer)
{
	commtimer_stop(timer);
	close(timer->tmfd);
	return 0;
}

