#pragma once
#include <stdarg.h>

#include "evcoro_scheduler.h"

/*
 * 当调用入evcoro_transfer_wait的第一个evcoro_swapswitch后，
 * 如果内部又调evcoro_transfer_goto切到第三个线程调度器上，
 * 那下面的evcoro_swapswitch内的_scheduler_checkowner会检测失败，
 * 所以evcoro_transfer_goto和evcoro_transfer_wait不能共存.
 */
#if 1
void evcoro_transfer_goto(struct evcoro_scheduler *from, struct evcoro_scheduler *gvto);
#else
typedef void (*TRANSFER_WORK_WAIT)(struct evcoro_scheduler *scheduler, va_list ap);

void evcoro_transfer_wait(struct evcoro_scheduler *from, struct evcoro_scheduler *gvto, TRANSFER_WORK_WAIT work, ...);

#define	EVCORO_TRANSFER_WAIT_RUN(scheduler, ap)	\
	{	\
		void __work_wait(struct evcoro_scheduler *scheduler, va_list ap)	\
		{


#define	EVCORO_TRANSFER_WAIT_END(from, gvto, ...)	\
		}	\
		evcoro_transfer_wait((struct evcoro_scheduler *)from, (struct evcoro_scheduler *)gvto, __work_wait, ##__VA_ARGS__);	\
	}
#endif


typedef void (*TRANSFER_WORK_NOWAIT)(struct evcoro_scheduler *scheduler, void *usr);

void evcoro_transfer_nowait(struct evcoro_scheduler *from, struct evcoro_scheduler *gvto, TRANSFER_WORK_NOWAIT work, void *usr);


/*函数外的栈变量不安全*/
#define	EVCORO_TRANSFER_NOWAIT_RUN(scheduler, usr)	\
	{	\
		void __work_nowait(struct evcoro_scheduler *scheduler, void *usr)	\
		{


#define	EVCORO_TRANSFER_NOWAIT_END(from, gvto, usr)	\
		}	\
		evcoro_transfer_nowait((struct evcoro_scheduler *)from, (struct evcoro_scheduler *)gvto, __work_nowait, (void *)usr);	\
	}
