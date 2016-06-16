/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/08.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_lock.h"
#include <time.h>

static void _fill_absolute_time(struct timespec *tmspec, long value);

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

/* @返回值:true 等到的条件成立，即@addr地址的上已经被改变为@value, false：等待条件未成立*/
bool commlock_wait(struct comm_lock *commlock, int *addr, int value, int timeout, bool locked)
{
	assert(commlock && commlock->init && addr);

	bool flag = true;

	if (!locked) {
		/* 调用此函数之前此锁未被调用该函数的函数锁住 */
		if (pthread_mutex_lock(&commlock->mutex) == -1) {
			return false;
		}
	}

	if (*addr != value) {
		if (timeout > 0) {
			struct timespec tm = {};
			_fill_absolute_time(&tm, timeout);
			/* 设置了超时等待，则选择带超时功能设置的条件变量等待函数 */
			pthread_cond_timedwait(&commlock->cond, &commlock->mutex, &tm);
		} else {
			pthread_cond_wait(&commlock->cond, &commlock->mutex);
		}

		if (*addr == value) {
			flag = true;
		} else {
			flag = false;
		}
	}

	if (!locked) {
		pthread_mutex_unlock(&commlock->mutex);
	}

	return flag;
}

bool commlock_wake(struct comm_lock *commlock, int *addr, int value, bool locked)
{
	assert(commlock && commlock->init && addr);

	int     old = -1;
	bool    flag = false;

	if (!locked) {
		/* 调用此函数之前此锁未被调用该函数的函数锁住 */
		if (pthread_mutex_lock(&commlock->mutex) == -1) {
			return flag;
		}
	}

	/* 原子设置@addr地址上的值为@value*/
	old = ATOMIC_SWAP(addr, value);

	if (likely(old != value)) {
		/* 返回的值为@addr地址上的旧值，说明设置成功,唤醒等待线程 */
		pthread_cond_broadcast(&commlock->cond);
		flag = true;
	}

	if (!locked) {
		pthread_mutex_unlock(&commlock->mutex);
	}

	return flag;
}

/* 填充绝对时间，精度为毫秒 */
static void _fill_absolute_time(struct timespec *tmspec, long value)
{
	assert(tmspec);

	clock_gettime(CLOCK_REALTIME, tmspec);

	tmspec->tv_sec += value / 1000;
	tmspec->tv_nsec = (long)((value % 1000) * 1000000);

	if (tmspec->tv_nsec > 1000000000) {
		tmspec->tv_sec += 1;
		tmspec->tv_nsec -= 1000000000;
	}
}

