#pragma once
#include <stdbool.h>

#include "core/evcs_events.h"
#include "swl_evcs.h"

extern struct evcs_events g_swl_evcs_evts;

struct swl_evcs_argv_settings
{
	unsigned int    tszp;
	void            *data;

	bool            (*task_lookup)(void *data, void *addr);
	bool            (*task_report)(void *data, void *addr);
	void            (*task_handle)(struct swell_task_info *info);
};

void swl_evcs_init(void *self);

void swl_evcs_exit(void *self);

