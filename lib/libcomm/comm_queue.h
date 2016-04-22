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



struct comm_queue{
	bool	init;		/* 结构体是否初始化的标志 */
	int	capacity;	/* 队列里面能存放节点的总数 */
	int	nodes;		/* 已存放的节点个数 */
	int	nodesize;	/* 单个节点的大小 */
	int	headidx;	/* 队列首节点的索引 */
	int	tailidx;	/* 队列尾节点的索引 */
	int 	readable;	/* 队列中是否有数据可读 只有用锁的时候才启用此变量 0 不可读 1 可读 */
	int	writeable;	/* 队列中是否有空间可写 只有用锁的时候才启用此变量 0 不可写 2 可写 */
	char*	queue;		/* 队列存储地址 */
};


/* 创建一个队列 @nodesize：单个节点的大小，即需要保存的一个数据包的大小 */
bool commqueue_init(struct comm_queue* comm_queue,  int capacity, int nodesize);

/* push一个节点的数据到queue里面 */
bool commqueue_push(struct comm_queue* comm_queue, const void* data);

/* 从queue里面pull一个节点的数据出来 */
bool commqueue_pull(struct comm_queue* comm_queue, void* data);

/* 销毁queue */
void commqueue_destroy(struct comm_queue* comm_queue);



#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_QUEUE_H__ */
