#pragma once
#include <stdbool.h>

#include "core/evcs_events.h"
#include "spx_evcs.h"

extern struct evcs_events g_spx_evcs_evts;

struct spx_evcs_argv_settings
{
	unsigned int    num;
	unsigned int    tsz;
	void            *data;

	bool            (*task_lookup)(void *data, void *addr);
	bool            (*task_report)(void *data, void *addr);
	void            (*task_handle)(struct supex_evcoro *evcoro, int step);
};

void spx_evcs_init(void *self);

void spx_evcs_exit(void *self);

