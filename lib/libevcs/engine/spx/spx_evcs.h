#pragma once

#include "../base/utils.h"
#include "evcoro_scheduler.h"

/* ------------                            */
/* evcoro 模式 */
struct supex_evcoro
{
	unsigned int            num;
	unsigned int            tsz;
	void                    *task;
	void                    *data;
	void                    *temp;

	union virtual_system    *VMS;
	bool                    (*task_lookup)(void *data, void *addr);
	bool                    (*task_report)(void *data, void *addr); /** 内部使用,自己给自己提供任务 */
	void                    (*task_handle)(struct supex_evcoro *evcoro, int step);//TODO:FIXME

	struct evcoro_scheduler *scheduler;	/** 协程集群句柄 */
};

void supex_evcoro_loop(struct supex_evcoro *evcoro);

/* ------------                            */

void supex_evcoro_init(struct supex_evcoro *evcoro);

void supex_evcoro_ring(struct supex_evcoro *evcoro);

void supex_evcoro_once(struct supex_evcoro *evcoro);

void supex_evcoro_exit(struct supex_evcoro *evcoro);

struct supex_evcoro     *supex_get_default(void);

