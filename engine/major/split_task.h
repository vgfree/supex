#pragma once
#include "scco.h"
#include "utils.h"
#include "def_task.h"

#define MAX_SPLIT_HTTP_NUMBER   MAX_LIMIT_FD
#define MAX_SPLIT_MSMQ_NUMBER   5
#define MAX_SPLIT_PLAN_NUMBER   1
#define MAX_SPLIT_QUEUE_NUMBER  (MAX_SPLIT_HTTP_NUMBER + MAX_SPLIT_MSMQ_NUMBER + MAX_SPLIT_PLAN_NUMBER)

/**
 * 任务模型
 */
struct split_task_node
{
	int                     id;	/**< 任务ID，任务ID在任务属性池中对应一组任务额外属性*/
	int                     sfd;	/**< 套接字描述符*/

	char                    type;	/**< 任务类型：线程池中所有线程响应、单个线程响应*/
	char                    origin;	/**< 任务来源：框架内部、外部接口*/

	int                     index;
	char                    *deal;	/**< 完成标志，指向调用层的局部变量的指针*/
	void                    *data;	/**< 任务内部资源*/
	SUPEX_TASK_CALLBACK     func;	/**< 任务回调*/
};

/*---------------------------------------------------------*/

int split_task_rgst(char origin, char type, short workers, short taskers, int mark);

int split_task_over(int id);

void split_task_come(int *store, int id);

int split_task_last(int *store, int id);

