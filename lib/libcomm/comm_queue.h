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

/* 销毁队列里面数据的回调函数原型 */
typedef void (*DestroyCB)(void *arg);

struct comm_queue
{
	bool            init;		/* 结构体是否初始化的标志 */
	int             capacity;	/* 队列里面能存放节点的总数 */
	int             nodes;		/* 已存放的节点个数 */
	int             nodesize;	/* 单个节点的大小 */
	int             headidx;	/* 队列首节点的索引 */
	int             tailidx;	/* 队列尾节点的索引 */
	int             readable;	/* 队列是否可读,用锁时才需要用此变量,需要自己管理此变量 */
	int             writeable;	/* 队列是否可写,用锁时才需要用此变量,需要自己管理此变量 */
	char            *buffer;	/* 队列存储数据缓冲区 */
	DestroyCB       destroy;	/* 销毁队列里面数据的回调函数 */
};

/* 创建一个队列 @nodesize：单个节点的大小 @capacity:队列能保存多少个节点 @destroy:销毁队列里面数据的回调函数 */
bool commqueue_init(struct comm_queue *comm_queue, int nodesize, int capacity, DestroyCB destroy);

/* 往队列里面添加一个节点的数据 @data:待添加的节点数据*/
bool commqueue_push(struct comm_queue *comm_queue, const void *data);

/* 从队列里面取出一个节点的数据 @data:用于保存取出的节点数据*/
bool commqueue_pull(struct comm_queue *comm_queue, void *data);

/* 销毁队列 */
void commqueue_destroy(struct comm_queue *comm_queue);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __COMM_QUEUE_H__ */

