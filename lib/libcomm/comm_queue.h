/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/17.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_QUEUE_H__
#define __COMM_QUEUE_H__

#include "comm_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRAVEL_FCB
typedef void (*TRAVEL_FCB)(const void *data, size_t size, size_t idx, void *usr);
#endif


#ifndef AO_T
typedef volatile long AO_T;
#endif
#ifndef AO_INC
  #define AO_INC(ptr)   ((__typeof__(*(ptr)))__sync_add_and_fetch((ptr), (1)))
#endif
#ifndef AO_DEC
  #define AO_DEC(ptr)   ((__typeof__(*(ptr)))__sync_sub_and_fetch((ptr), (1)))
#endif

/** 任务队列*/
struct comm_queue
{
	bool                    init;		/* 结构体是否初始化的标志 */
	unsigned int            capacity;	/* 队列里面能存放节点的总数 */
	unsigned int            all;		/* 最大空间 */
	unsigned int            dsz;		/* 单个节点的大小 */
	unsigned int            isz;		/* 累计压入 */
	unsigned int            osz;		/* 累计弹出 */
	AO_T                    nodes;		/* 已存放的节点个数,nodes > 0 时代表有新增任务 */
	void                    *slots;		/* 队列存储数据缓冲区 */

	unsigned int            headidx;	/* 队列首节点的索引 */
	unsigned int            tailidx;	/* 队列尾节点的索引 */

	pthread_spinlock_t      w_lock;		/* 压入锁 */
	pthread_spinlock_t      r_lock;		/* 弹出锁 */
};

/**
 * 初始化
 * @param list 被初始化队列
 * @param dsz  元素大小
 * @param capacity  最大容量
 * @destroy:销毁队列里面数据的回调函数
 */
bool commqueue_init(struct comm_queue *list, unsigned int dsz, unsigned int capacity);

/**
 * 将元素的副本压入队列尾部
 * @param list 操作队列
 * @param data 被压入队列的元素
 * @return 是否推入成功true/false
 */
bool commqueue_push(struct comm_queue *list, void *data);

/**
 * 从队列的头部弹出一个元素的副本
 * @param list 操作队列
 * @param data 被弹出队列的元素
 * @return 是否获取成功true/false
 */
bool commqueue_pull(struct comm_queue *list, void *data);

/**
 * 查看队列头数据
 */
bool commqueue_top(struct comm_queue *list, void *data);

/**
 * 遍历队列
 */
void commqueue_travel(struct comm_queue *list, bool (*travel)(void *, size_t size, void *), void *data);

/* 销毁队列 */
void commqueue_destroy(struct comm_queue *list, TRAVEL_FCB fcb, void *usr);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __COMM_QUEUE_H__ */

