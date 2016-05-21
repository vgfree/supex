/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/17.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_queue.h"

#define MAXQUEUENODES	1024	/* 队列能存的最大节点数 */

bool commqueue_init(struct comm_queue* commqueue, int nodesize, int capacity, DestroyCB destroy)
{
	assert(commqueue && nodesize > 0);

	memset(commqueue, 0, sizeof(*commqueue));
	commqueue->capacity = (capacity <= 0 || capacity > MAXQUEUENODES) ? MAXQUEUENODES : capacity;
	commqueue->nodesize = nodesize;
	commqueue->destroy = destroy;
	commqueue->writeable = 1;
	commqueue->readable = 1;
	commqueue->buffer = calloc(commqueue->capacity, commqueue->nodesize);
	if (commqueue->buffer) {
		commqueue->init = true;
		return true;
	} else {
		return false;
	}
}


bool commqueue_push(struct comm_queue* commqueue, const void* data)
{
	assert(commqueue && commqueue->init && data);
	if (commqueue->nodes < commqueue->capacity) {
		memcpy(&commqueue->buffer[commqueue->tailidx * commqueue->nodesize ], data, commqueue->nodesize);
		commqueue->tailidx = (commqueue->tailidx + 1)%commqueue->capacity;
		commqueue->nodes += 1;
		return true;
	} else {
		return false;
	}
}

bool commqueue_pull(struct comm_queue* commqueue, void* data)
{
	assert(commqueue && commqueue->init && data);
	if (commqueue->nodes > 0) {
		int index = commqueue->headidx * commqueue->nodesize;
		memcpy(data, &commqueue->buffer[commqueue->headidx * commqueue->nodesize ], commqueue->nodesize);
		commqueue->headidx = (commqueue->headidx + 1)%commqueue->capacity;
		commqueue->nodes -= 1;
		return true;
	} else {
		return false;
	}
}


void commqueue_destroy(struct comm_queue* commqueue)
{
	void *data = NULL;
	if (commqueue && commqueue->init) {
		if ((commqueue->nodes > 0) && commqueue->destroy) {
			while(commqueue->nodes) {
				commqueue_pull(commqueue, (void*)&data);
				commqueue->destroy(data);
			}
		}
		Free(commqueue->buffer);
		commqueue->init = false;
	}
}
