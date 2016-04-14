/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/08.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_lock.h"

static void _fill_absolute_time(struct timespec *tmspec, long value);

bool commlock_init(struct comm_lock *commlock)
{
	int retval = -1;
	if ((retval = pthread_mutex_init(&commlock->mutex, NULL)) != 0) {
		return false;
	}
	pthread_condattr_t attr;
	pthread_condattr_init(&attr);
	pthread_condattr_setclock(&attr, CLOCK_MONOTONIC); /* 防止pthread_cond_timewait()出现意外 */
	if ((retval = pthread_cond_init(&commlock->cond, &attr)) != 0) {
		pthread_condattr_destroy(&attr);
		pthread_mutex_destroy(&commlock->mutex);
		return false;
	}
	pthread_condattr_destroy(&attr);
	commlock->available = true;
	commlock->waiters = 0;
	return true;
}


bool commlock_lock(struct comm_lock *commlock, bool condable)
{
	return pthread_mutex_lock(&commlock->mutex) == 0;
}

bool commlock_trylock(struct comm_lock *commlock)
{
	return pthread_mutex_trylock(&commlock->mutex) == 0;
}


bool commlock_unlock(struct comm_lock *commlock)
{
	return pthread_mutex_unlock(&commlock->mutex) == 0;
}

bool  commlock_destroy(struct comm_lock *commlock)
{
	if ( commlock->available == false || commlock->waiters > 0) {
		return false;
	}
	commlock->waiters = 0;
	commlock->available = true;
	pthread_mutex_destroy(&commlock->mutex);
	pthread_cond_destroy(&commlock->cond);
	return true;
}


/* 等待@addr 地址上的值变为 @value之后或 timeout之后返回 */
bool commlock_wait(struct comm_lock *commlock, int *addr,  int value, int timeout)
{
	int	retval = -1;
	bool	flag = true;
	retval = pthread_mutex_lock(&commlock->mutex);
	if (retval != -1) {
		if (*addr == value) {
			flag = true;
		}else {
			if (timeout > 0) {
				struct timespec tm = {};
				_fill_absolute_time(&tm, timeout);
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
		pthread_mutex_unlock(&commlock->mutex);
	} else {
		flag = false;
	}

	return flag;
}

/* 设置@*addr的值并唤醒条件等待的线程 */
bool commlock_wake(struct comm_lock *commlock, int *addr,  int value)
{
	int retval = -1;
	int old =  -1;
	retval = pthread_mutex_lock(&commlock->mutex);
	if (retval == -1) {
		return false;
	}
	old = ATOMIC_SWAP(addr, value);
	if (likely(old != value)) { /* 返回的值不等于新值，说明设置成功 */
		pthread_cond_broadcast(&commlock->cond);
		retval = -1;
	}
	pthread_mutex_unlock(&commlock->mutex);
	return retval != -1;
}


/* 填充绝对时间，精度为毫秒 */
static void _fill_absolute_time(struct timespec *tmspec, long value)
{
	assert(tmspec);

	struct timeval tv = {};

	gettimeofday(&tv, NULL);

	tv.tv_sec += value / 1000;
	tv.tv_usec += (value % 1000) * 1000;

	if (tv.tv_usec > 1000000) {
		tv.tv_sec += 1;
		tv.tv_usec -= 1000000;
	}

	tmspec->tv_sec = tv.tv_sec;
	tmspec->tv_nsec = (long)(tv.tv_usec * 1000);
}
