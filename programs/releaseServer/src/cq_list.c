/*
 *   ====================================================================
 *   模块说明：
 *        本模块提供一个公共的循环队列api
 *        cycle queue
 *
 *   应用：
 *        此队列在本项目中属于线程的一个字段，用户线程接收任务
 *   ====================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cq_list.h"

#define CQ_LOCK         1	/*原子上锁*/
#define CQ_UNLOCK       0

typedef struct _CQ
{
	void    *cq[CQ_LEN + 1];
	int     head;
	int     rear;
	int     lock;
} CQ;

/*
 *功能：初始化一个队列
 *出参：
 *	cq:队列句柄
 *返回值：
 *	0：
 *	-1：
 */
int cqu_init(void **cq)
{
	if (NULL == cq) {
		return -1;
	}

	CQ *q;

	*cq = calloc(1, sizeof(CQ));

	if (NULL == *cq) {
		return -1;
	}

	q = (CQ *)*cq;
	q->head = 0;
	q->rear = 0;
	q->lock = 0;

	return 0;
}

static int cqu_isempty(void *cq)
{
	CQ *q;

	q = (CQ *)cq;

	if (q->head == q->rear) {
		return 1;
	}

	return 0;
}

static int cqu_isfull(void *cq)
{
	CQ *q;

	q = (CQ *)cq;

	if ((q->rear + 1) % (CQ_LEN + 1) == q->head) {
		return 1;
	}

	return 0;
}

/*
 *功能：将数据加入队列
 * 入参：
 *	cq:队列句柄
 *	data:数据
 *
 *返回值：
 *	0
 *	-1
 * 说明：
 *      data = malloc()
 * */
int cqu_push(void *cq, void *data)
{
	CQ *q;

	q = (CQ *)cq;

	while (!__sync_bool_compare_and_swap(&q->lock, CQ_UNLOCK, CQ_LOCK)) {}

	if (cqu_isfull(q)) {
		assert(__sync_bool_compare_and_swap(&q->lock, CQ_LOCK, CQ_UNLOCK));
		return -1;
	}

	q->cq[q->rear] = data;
	q->rear = (q->rear + 1) % (CQ_LEN + 1);
	assert(__sync_bool_compare_and_swap(&q->lock, CQ_LOCK, CQ_UNLOCK));
	return 0;
}

/*
 *功能：从队列中取数据
 * 入参：
 *	cq
 *出参：
 *	data:数据
 *返回值：
 *	-1
 *	0
 *说明：
 *	外部free(*data);
 * */
int cqu_pull(void *cq, void **data)
{
	CQ *q;

	q = (CQ *)cq;

	while (!__sync_bool_compare_and_swap(&q->lock, CQ_UNLOCK, CQ_LOCK)) {}

	if (cqu_isempty(q)) {
		assert(__sync_bool_compare_and_swap(&q->lock, CQ_LOCK, CQ_UNLOCK));
		return -1;
	}

	*data = q->cq[q->head];
	q->head = (q->head + 1) % (CQ_LEN + 1);
	assert(__sync_bool_compare_and_swap(&q->lock, CQ_LOCK, CQ_UNLOCK));

	return 0;
}

