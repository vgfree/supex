/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/08.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_LOCK_H__
#define __COMM_LOCK_H__

#include "comm_utils.h"
#include <pthread.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif


     
struct comm_lock{
	int		waiters;	/* 锁的等待者个数 */
	bool		available;	/* 锁是否空闲可用 */
	bool		condable;	/* 是否是条件上锁 */
	pthread_mutex_t mutex;
	pthread_cond_t	cond;
};

/* 初始化锁 */
bool commlock_init(struct comm_lock *commlock);

/* 获取锁并锁住，如果锁被占用则阻塞到锁可用 */
bool commlock_lock(struct comm_lock *commlock, bool condable);

/* 尝试获取锁，如果锁被占用则立即返回 */
bool commlock_trylock(struct comm_lock *commlock);

/* 解锁 */
bool commlock_unlock(struct comm_lock *commlock);

bool commlock_wait_cond(struct comm_lock *commlock, int cond, int timeout);

bool commlock_wait(struct comm_lock *commlock, int *addr,  int value, int timeout);

/* 销毁锁 */
bool commlock_destroy(struct comm_lock *commlock);


     
#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_LOCK_H__ */
