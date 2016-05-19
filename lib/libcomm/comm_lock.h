/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/08.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_LOCK_H__
#define __COMM_LOCK_H__

#include "comm_utils.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif


     
struct comm_lock{
	bool		init;		/* 锁的初始化标志，1为已初始化 */
	pthread_mutex_t mutex;		/* 互斥量 */
	pthread_cond_t	cond;		/* 条件变量 */
};

/* 初始化锁 */
bool commlock_init(struct comm_lock *commlock);

/* 销毁锁 */
void commlock_destroy(struct comm_lock *commlock);

/* 获取锁并锁住，如果锁被占用则阻塞到锁可用 */
bool commlock_lock(struct comm_lock *commlock);

/* 尝试获取锁，如果锁被占用则立即返回 */
bool commlock_trylock(struct comm_lock *commlock);

/* 解锁 */
bool commlock_unlock(struct comm_lock *commlock);

/* 等待@addr地址上的值变为@value或者@timeout超时返回，如果超时没有设置，则阻塞等待 @locked:调用此函数之前是否已经锁住此锁 */
bool commlock_wait(struct comm_lock *commlock, int *addr,  int value, int timeout, bool locked);

/* 设置@addr地址上的值为@value并唤醒等待线程 @locked:在调用此函数之前是否已经锁住此锁 */
bool commlock_wake(struct comm_lock *commlock, int *addr,  int value, bool locked);


     
#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_LOCK_H__ */
