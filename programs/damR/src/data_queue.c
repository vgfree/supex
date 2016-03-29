//
//  data_queue.c
//  supex
//
//  Created by 周凯 on 15/9/16.
//  Copyright © 2015年 zk. All rights reserved.
//
#include "zmq_wrap.h"
#include "data_queue.h"
/* --------             */
#define EXCEPT_CREATE_QUEUE_CODE (0x00000001)

static const ExceptT EXCEPT_CREATE_QUEUE = {
	"can't create queue.",
	EXCEPT_CREATE_QUEUE_CODE,
};

/* --------             */
struct queue g_queue;
/* --------             */

void queue_init(struct queue *queue, struct cfg *cfg)
{
	assert(queue);

	ASSERTOBJ(cfg);

	TRY
	{
		memset(queue, 0, sizeof(*queue));
		snprintf((char *)queue->name, sizeof(queue->name), "%s/%08d.dat", cfg->queue.path, cfg->queue.seq);

		switch (cfg->queue.type)
		{
			case FILE_QUEUE:
			{
				queue->push = (queue_pushpull_cb)File_QueuePush;
				queue->pull = (queue_pushpull_cb)File_QueuePull;
				queue->priopush = (queue_pushpull_cb)File_QueuePriorityPush;

				queue->create = (queue_create_cb)File_QueueCreate;
				queue->destroy = (queue_destroy_cb)File_QueueDestroy;
			}
			break;

			case SHM_QUEUE:
			{
				queue->push = (queue_pushpull_cb)SHM_QueuePush;
				queue->pull = (queue_pushpull_cb)SHM_QueuePull;
				queue->priopush = (queue_pushpull_cb)SHM_QueuePriorityPush;

				queue->create = (queue_create_cb)SHM_QueueCreateByPath;
				queue->destroy = (queue_destroy_cb)SHM_QueueDestroy;
			}
			break;

			case MEM_QUEUE:
			{
				queue->push = (queue_pushpull_cb)MEM_QueuePush;
				queue->pull = (queue_pushpull_cb)MEM_QueuePull;
				queue->priopush = (queue_pushpull_cb)MEM_QueuePriorityPush;

				queue->create = (queue_create_cb)MEM_QueueCreate;
				queue->destroy = (queue_destroy_cb)MEM_QueueDestroy;
			}
			break;

			default:
				RAISE(EXCEPT_ASSERT);
				break;
		}

		int cellsize = offsetof(struct zmqframe, data) + cfg->queue.cellsize;
		queue->queue = queue->create((char *)queue->name, cfg->queue.capacity, cellsize);
		AssertRaise(queue->queue, EXCEPT_CREATE_QUEUE);

		switch (cfg->queue.type)
		{
			case FILE_QUEUE:
			{
				FileQueueT queueimp = (FileQueueT)queue->queue;
				ASSERTOBJ(queueimp);
				queue->userspace = queueimp->data->usr;
				queue->nodes = &queueimp->data->nodes;
				queue->cellsize = queueimp->data->datasize;
				queue->capacity = queueimp->data->capacity;
			}
			break;

			case SHM_QUEUE:
			{
				ShmQueueT queueimp = (ShmQueueT)queue->queue;
				ASSERTOBJ(queueimp);
				queue->userspace = queueimp->data->usr;
				queue->nodes = &queueimp->data->nodes;
				queue->cellsize = queueimp->data->datasize;
				queue->capacity = queueimp->data->capacity;
			}
			break;

			default:
			{
				MemQueueT queueimp = (MemQueueT)queue->queue;
				ASSERTOBJ(queueimp);
				queue->userspace = queueimp->data->usr;
				queue->nodes = &queueimp->data->nodes;
				queue->cellsize = queueimp->data->datasize;
				queue->capacity = queueimp->data->capacity;
			}
			break;
		}

		REFOBJ(queue);
	}
	CATCH
	{
		if (queue->destroy) {
			queue->destroy(&queue->queue, false, NULL);
		}

		RERAISE;
	}
	END;
}

void queue_finally(struct queue *queue)
{
	return_if_fail(UNREFOBJ(queue));

	queue->destroy(&queue->queue, false, NULL);
}

/*
 * 检查队列是否满或空
 */
void queue_check_isfull(struct queue *queue)
{
	ASSERTOBJ(queue);

	do {
		int flag = 0;

		flag = futex_wait(queue->nodes, queue->capacity, 200);

		if (likely(flag != 0)) {
			break;
		}
	} while (0);
}

void queue_check_isempty(struct queue *queue)
{
	ASSERTOBJ(queue);

	do {
		int flag = 0;

		flag = futex_wait(queue->nodes, 0, 200);

		if (likely(flag != 0)) {
			break;
		}
	} while (0);
}

