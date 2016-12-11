#pragma once

#include "../base/utils.h"
#include "evcoro_scheduler.h"


/* ------------                            */
struct swell_task_info
{
	struct swell_evcoro     *evcoro;
	union virtual_system    VMS;

	unsigned int            tsize;
	void                    *tdata;
};

/* evcoro 模式 */
struct swell_evcoro
{
	void                    *data;
	unsigned int            tszp;		/** task size peak */
	int			tefd;		/** task event fd */
	ev_io                   task_watcher;

	bool                    (*task_lookup)(void *data, void *addr);	/** 通过tefd来读取数据长度,因此此接口要严格保证fifo */
	bool                    (*task_report)(void *data, void *addr); /** 内部使用,自己给自己提供任务 */
	void                    (*task_handle)(struct swell_task_info *info);

	struct evcoro_scheduler *scheduler;	/** 协程集群句柄 */
};

void swell_evcoro_loop(struct swell_evcoro *evcoro);

/* ------------                            */

void swell_evcoro_init(struct swell_evcoro *evcoro);

void swell_evcoro_ring(struct swell_evcoro *evcoro);

void swell_evcoro_once(struct swell_evcoro *evcoro);

void swell_evcoro_exit(struct swell_evcoro *evcoro);

struct swell_evcoro     *swell_get_default(void);

int eventfd_xsend(int efd, eventfd_t value);
