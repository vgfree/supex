#pragma once
#include <stdarg.h>

#include "evcoro_scheduler.h"

typedef void (*TRANSFER_WORK_WAIT)(struct evcoro_scheduler *scheduler, va_list ap);
typedef void (*TRANSFER_WORK_NOWAIT)(struct evcoro_scheduler *scheduler, void *usr);

void evcoro_transfer_wait(struct evcoro_scheduler *from, struct evcoro_scheduler *gvto, TRANSFER_WORK_WAIT work, ...);

void evcoro_transfer_nowait(struct evcoro_scheduler *from, struct evcoro_scheduler *gvto, TRANSFER_WORK_NOWAIT work, void *usr);


#define	EVCORO_TRANSFER_WAIT_RUN(scheduler, ap)	\
	{	\
		void __work_wait(struct evcoro_scheduler *scheduler, va_list ap)	\
		{


#define	EVCORO_TRANSFER_WAIT_END(from, gvto, ...)	\
		}	\
		evcoro_transfer_wait((struct evcoro_scheduler *)from, (struct evcoro_scheduler *)gvto, __work_wait, ##__VA_ARGS__);	\
	}


#define	EVCORO_TRANSFER_NOWAIT_RUN(scheduler, usr)	\
	{	\
		void __work_nowait(struct evcoro_scheduler *scheduler, void *usr)	\
		{


#define	EVCORO_TRANSFER_NOWAIT_END(from, gvto, usr)	\
		}	\
		evcoro_transfer_nowait((struct evcoro_scheduler *)from, (struct evcoro_scheduler *)gvto, __work_nowait, (void *)usr);	\
	}
