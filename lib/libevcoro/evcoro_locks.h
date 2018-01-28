#pragma once
#include <pthread.h>

#include "libevcoro.h"
#include "qlist.h"


#define ONLY_WAKE_ONE
/*
 * 全部唤醒的情况是:
 * 	当trylock协程得不到锁进入休眠,
 * 	再被其它unlock协程唤醒,
 * 	被唤醒后的协程不再去获取锁,
 * 	这样还在等待锁的休眠协程无法被唤醒.
 */

enum evcoro_lock_type
{
	MUTEX_LOCK_TYPE = 0,
	SPIN_LOCK_TYPE,
	RW_LOCK_TYPE,
};

enum evcoro_lock_opts
{
	LOCK_NO_OPTS = -1,
	LOCK_RD_OPTS = 0,
	LOCK_WT_OPTS = 1,
};


typedef struct evcoro_locks {
	union {
		pthread_mutex_t mutex;
		pthread_spinlock_t spin;
		pthread_rwlock_t rw;
	} 		base;
	enum evcoro_lock_type type;

	enum evcoro_lock_opts rw_opts;	/*记录锁所在的是读还是写状态,未使用,保留*/
	AO_T		wt_cnts;

	QLIST		list;
} evcoro_locks_t;

#define EVCORO_MUTEX_INITIALIZER \
	{	\
		.base = {.mutex = PTHREAD_MUTEX_INITIALIZER},	\
		.type = MUTEX_LOCK_TYPE,	\
		.rw_opts = LOCK_NO_OPTS,	\
		.wt_cnts = 0,	\
		.list = {.lock = PTHREAD_MUTEX_INITIALIZER}	\
	}

#define EVCORO_RWLOCK_INITIALIZER \
	{	\
		.base = {.rw = PTHREAD_RWLOCK_INITIALIZER},	\
		.type = RW_LOCK_TYPE,	\
		.rw_opts = LOCK_NO_OPTS,	\
		.wt_cnts = 0,	\
		.list = {.lock = PTHREAD_MUTEX_INITIALIZER}	\
	}


int evcoro_locks_init(evcoro_locks_t *restrict locks, void *attr, enum evcoro_lock_type type);

int evcoro_locks_destroy(evcoro_locks_t *locks);

/*rw_opts: 0 is read and 1 is write, only pthread_rwlock_t active!*/
int evcoro_locks_trylock(struct evcoro_scheduler *scheduler, evcoro_locks_t *locks, enum evcoro_lock_opts rw_opts);

int evcoro_locks_lock(struct evcoro_scheduler *scheduler, evcoro_locks_t *locks, enum evcoro_lock_opts rw_opts);

int evcoro_locks_unlock(struct evcoro_scheduler *scheduler, evcoro_locks_t *locks);
