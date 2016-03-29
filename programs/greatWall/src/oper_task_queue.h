//
//  oper_task_queue.h
//  supex
//
//  Created by 周凯 on 15/9/12.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef oper_task_queue_h
#define oper_task_queue_h

#include <stdio.h>
#include "utils.h"

__BEGIN_DECLS

enum ext_task_type
{
	ETT_NONE,
	ETT_SHMQ,
	ETT_FILEQ,
};

struct ext_tqueue;

typedef void *(*ext_tqueue_create_cb)(struct ext_tqueue *etqueue, void *name);
typedef bool (*ext_tqueue_oper_cb)(void *queue, char *buff, int size, int *effectsize);

struct ext_tqueue_id
{
	void    *name;
	void    *queue;
};

struct ext_tqueue
{
	int                     magic;
	/**队列类型*/
	enum ext_task_type      type;
	/**队列名称提示*/
	intptr_t                namehint;
	/**队列路径*/
	char                    *path;
	/**队列池*/
	SListT                  qpool;

	NodeCompareCB           findq;

	ext_tqueue_create_cb    createq;
	ext_tqueue_oper_cb      pushq;
	ext_tqueue_oper_cb      pushprioq;
	ext_tqueue_oper_cb      pullq;

	int                     qtotalnodes;
	int                     qnodesize;
};

extern struct ext_tqueue g_ext_tqueue;

bool ext_tqueue_init(struct ext_tqueue *tqueue, const char *type, const char *hint, const char *path);

bool ext_tqueue_push(struct ext_tqueue *tqueue, const char *hint, const char *data, int size);

bool ext_tqueue_pull(struct ext_tqueue *tqueue, const char *hint, const char *data, int size);

__END_DECLS
#endif	/* oper_task_queue_h */

