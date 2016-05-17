#pragma once
#include <stdbool.h>

#include "core/evcs_events.h"

struct evcs_events g_evcs_evts = {};

struct evcs_argv_settings
{
	unsigned int            num;
	unsigned int            tsz;
	void                    *data;

	bool                    (*task_lookup)(void *data, void *addr);
	void                    (*task_handle)(void *data, void *addr);
};


void evcs_init(void *self);

void evcs_exit(void *self);
