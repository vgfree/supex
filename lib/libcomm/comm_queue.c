/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/17.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_queue.h"

#define QUEUE_SIZE	1024	/* 队列里面可以存放1024个节点 */

bool commqueue_init(struct comm_queue* comm_queue, int capacity, int nodesize, DestroyCB destroy)
{
	assert(comm_queue);
	memset(comm_queue, 0, sizeof(*comm_queue));
	comm_queue->capacity = capacity > 0 ? capacity : QUEUE_SIZE;
	comm_queue->nodesize = nodesize;
	comm_queue->destroy = destroy;
	comm_queue->writeable = 1;
	comm_queue->readable = 1;
	comm_queue->queue = calloc(capacity, nodesize);
	if (unlikely(!comm_queue->queue)) {
		return false;
	} else {
		comm_queue->init = true;
		return true;
	}
}


bool commqueue_push(struct comm_queue* comm_queue, const void* data)
{
	assert(comm_queue && comm_queue->init && data);
	if (likely(comm_queue->nodes < comm_queue->capacity)) {
		memcpy(&comm_queue->queue[comm_queue->tailidx * comm_queue->nodesize ], data, comm_queue->nodesize);
		comm_queue->tailidx = (comm_queue->tailidx + 1)%comm_queue->capacity;
		comm_queue->nodes += 1;
		return true;
	} else {
		return false;
	}
}

bool commqueue_pull(struct comm_queue* comm_queue, void* data)
{
	assert(comm_queue && comm_queue->init && data);
	if (likely(comm_queue->nodes > 0)) {
		int index = comm_queue->headidx * comm_queue->nodesize;
		memcpy(data, &comm_queue->queue[comm_queue->headidx * comm_queue->nodesize ], comm_queue->nodesize);
		comm_queue->headidx = (comm_queue->headidx + 1)%comm_queue->capacity;
		comm_queue->nodes -= 1;
		return true;
	} else {
		return false;
	}
}


void commqueue_destroy(struct comm_queue* comm_queue)
{
	void *data = NULL;
	if (likely(comm_queue && comm_queue->init)) {
		if (comm_queue->destroy && (comm_queue->nodes > 0)) {
			while(comm_queue->nodes) {
				commqueue_pull(comm_queue, (void*)&data);
				comm_queue->destroy(data);
			}
		}
		Free(comm_queue->queue);
	}
}
