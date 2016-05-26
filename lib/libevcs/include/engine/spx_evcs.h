#pragma once

#include "evcoro_async_tasks.h"

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
	void                    (*task_handle)(void *data, void *addr);

	struct evcoro_scheduler *scheduler;	/** 协程集群句柄 */
};

void supex_evcoro_loop(struct supex_evcoro *evcoro);

/* ------------                            */

void supex_evcoro_init(struct supex_evcoro *evcoro);

void supex_evcoro_ring(struct supex_evcoro *evcoro);

void supex_evcoro_once(struct supex_evcoro *evcoro);

void supex_evcoro_exit(struct supex_evcoro *evcoro);

struct supex_evcoro     *supex_get_default(void);

