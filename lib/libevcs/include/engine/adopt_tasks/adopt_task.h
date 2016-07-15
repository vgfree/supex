#pragma once
#include "../base/utils.h"

#define MAX_ADOPT_HTTP_NUMBER   MAX_LIMIT_FD
#define MAX_ADOPT_MSMQ_NUMBER   MAX_CONNECT
#define MAX_ADOPT_PLAN_NUMBER   (1 + 9999)
#define MAX_ADOPT_QUEUE_NUMBER  (MAX_ADOPT_HTTP_NUMBER + MAX_ADOPT_MSMQ_NUMBER + MAX_ADOPT_PLAN_NUMBER)

/*************************************************/
#define BIT8_TASK_TYPE_ALONE    'L'
#define BIT8_TASK_TYPE_WHOLE    'H'

#define BIT8_TASK_ORIGIN_HTTP   'p'
#define BIT8_TASK_ORIGIN_REDIS  'r'
#define BIT8_TASK_ORIGIN_MTTP   't'
#define BIT8_TASK_ORIGIN_MFPTP  'f'
#define BIT8_TASK_ORIGIN_MSMQ   'q'
#define BIT8_TASK_ORIGIN_TIME   'e'
#define BIT8_TASK_ORIGIN_INIT   'i'

#define TASK_ID_UNUSE           0		/**< 未用*/
#define TASK_ID_INUSE           1		/**< 在用*/

#define TASK_IS_BEGIN           0		/**< 已开始*/
#define TASK_IS_FINISH          1		/**< 已完成*/

struct share_task_view
{
	short   worker_affect;	/**< 该任务需要被完成多少次*/
	short   worker_finish;	/**< 处理线程已完成该任务的次数*/

	short   tasker_affect;	/**< 该任务需要被处理线程接收多少次*/
	short   tasker_finish;	/**< 处理线程已接收的次数*/

	short   cntl;		/**< 任务ID状态 ：TASK_ID_INUSE 、TASK_ID_UNUSE*/
};

/*************************************************/

/*==============================================================================================*
*       任务回调函数                                  *
*==============================================================================================*/
typedef int (*TASK_VMS_FCB)(void *user, union virtual_system **VMS, struct adopt_task_node *task);

/**
 * 任务模型
 */
struct adopt_task_node
{
	int             id;		/**< 任务ID，任务ID在任务属性池中对应一组任务额外属性*/

	uint64_t        cid;
	int             sfd;		/**< 套接字描述符*/

	char            type;		/**< 任务类型：线程池中所有线程响应、单个线程响应*/
	char            origin;		/**< 任务来源：框架内部、外部接口*/

	int             index;
	char            *deal;		/**< 完成标志，指向调用层的局部变量的指针*/
	TASK_VMS_FCB    func;		/**< 任务回调*/
	int             last;

	void            *data;		/**< 任务内部资源*/
	long            size;		/**< 任务内部资源*/
	bool            freeable;
};

/*---------------------------------------------------------*/

int adopt_task_rgst(char origin, char type, short workers, short taskers, int mark);

int adopt_task_over(int id);

void adopt_task_come(int *store, int id);

int adopt_task_last(int *store, int id);

