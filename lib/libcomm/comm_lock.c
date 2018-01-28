/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/08.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_lock.h"
#include <time.h>

bool commlock_init(struct comm_lock *commlock)
{
	if (pthread_mutex_init(&commlock->mutex, NULL) != 0) {
		return false;
	}

	if (pthread_cond_init(&commlock->cond, NULL) != 0) {
		pthread_mutex_destroy(&commlock->mutex);
		return false;
	}

	commlock->init = true;
	return true;
}

inline bool commlock_lock(struct comm_lock *commlock)
{
	assert(commlock && commlock->init);
	return pthread_mutex_lock(&commlock->mutex) == 0;
}

inline bool commlock_trylock(struct comm_lock *commlock)
{
	assert(commlock && commlock->init);
	return pthread_mutex_trylock(&commlock->mutex) == 0;
}

inline bool commlock_unlock(struct comm_lock *commlock)
{
	assert(commlock && commlock->init);
	return pthread_mutex_unlock(&commlock->mutex) == 0;
}

void commlock_destroy(struct comm_lock *commlock)
{
	if (commlock && commlock->init) {
		pthread_mutex_destroy(&commlock->mutex);
		pthread_cond_destroy(&commlock->cond);
		commlock->init = false;
	}
}

static void __pthread_exit_todo(void *data)
{
	pthread_mutex_unlock((pthread_mutex_t *)data);
}

/* 填充绝对时间，精度为毫秒 */
static void _fill_absolute_time(struct timespec *tmspec, long value)
{
	assert(tmspec);

	clock_gettime(CLOCK_REALTIME, tmspec);

	tmspec->tv_sec += value / 1000;
	tmspec->tv_nsec = (long)((value % 1000) * 1000000);
}

bool commlock_wait(struct comm_lock *commlock, int timeout)
{
	assert(commlock && commlock->init);

	pthread_cleanup_push(__pthread_exit_todo, &commlock->mutex);

	if (timeout > 0) {
		struct timespec tm = {};
		_fill_absolute_time(&tm, timeout);
		/* 设置了超时等待，则选择带超时功能设置的条件变量等待函数 */
		pthread_cond_timedwait(&commlock->cond, &commlock->mutex, &tm);
	} else {
		pthread_cond_wait(&commlock->cond, &commlock->mutex);
	}

	pthread_cleanup_pop(0);

	return true;
}

bool commlock_wake(struct comm_lock *commlock)
{
	assert(commlock && commlock->init);

	/* 返回的值为@addr地址上的旧值，说明设置成功,唤醒等待线程 */
	pthread_cond_broadcast(&commlock->cond);

	return true;
}

