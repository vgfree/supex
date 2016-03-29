/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/17.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef _COMM_QUEUE_H_
#define _COMM_QUEUE_H_

#include "comm_utils.h"

struct comm_queue{
	int capacity;	//队列里面能存放节点的总数
	int nodes;	//已存放的节点个数
	int nodesize;	//单个节点的大小
	int headidx;	//队列首节点的索引
	int tailidx;	//队列尾节点的索引
	char* queue;	//队列存储地址
};


/*创建一个队列 nodesize：单个节点的大小，即需要保存的一个数据包的大小*/
bool commqueue_init(struct comm_queue* comm_queue,  int capacity, int nodesize);

/*push一个节点的数据到queue里面*/
bool commqueue_push(struct comm_queue* comm_queue, const void* data);

/*从queue里面pull一个节点的数据出来*/
bool commqueue_pull(struct comm_queue* comm_queue, void* data);

/*销毁queue*/
void commqueue_destroy(struct comm_queue* comm_queue);

#endif
