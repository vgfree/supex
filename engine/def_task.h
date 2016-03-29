#pragma once

#define FREE_LOCK_QUEUE

#define BIT8_TASK_TYPE_ALONE    'L'
#define BIT8_TASK_TYPE_WHOLE    'H'

#define BIT8_TASK_ORIGIN_HTTP   'p'
#define BIT8_TASK_ORIGIN_REDIS  'r'
#define BIT8_TASK_ORIGIN_MSMQ   'q'
#define BIT8_TASK_ORIGIN_TIME   'e'
#define BIT8_TASK_ORIGIN_INIT   'i'

#ifdef _mttptest
  #define BIT8_TASK_ORIGIN_MTTP 'm'
#endif

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

