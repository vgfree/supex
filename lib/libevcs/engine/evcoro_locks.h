#pragma once

#include "base/qlist.h"
#include "evcoro_scheduler.h"


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


typedef struct evcoro_locks {
	union {
		pthread_mutex_t mutex;
		pthread_spinlock_t spin;
		pthread_rwlock_t rw;
	} 		base;
	enum evcoro_lock_type type;
	QLIST		list;
} evcoro_locks_t;




int evcoro_locks_init(evcoro_locks_t *restrict locks, void *attr, enum evcoro_lock_type type);

int evcoro_locks_destroy(evcoro_locks_t *locks);

/*opt: 0 is read and 1 is write*/
int evcoro_locks_trylock(struct evcoro_scheduler *scheduler, evcoro_locks_t *locks, int opt);

int evcoro_locks_lock(struct evcoro_scheduler *scheduler, evcoro_locks_t *locks, int opt);

int evcoro_locks_unlock(struct evcoro_scheduler *scheduler, evcoro_locks_t *locks);
