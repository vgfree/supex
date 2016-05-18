#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "evcs_module.h"
#include "evcs_events.h"

struct evcs_events g_kernel_evts = {};
	
static void kernel_start(void *argv)
{
	EVCS_MODULE_TOUCH( EVCS_EVENT_INIT );
	while (1) {
		EVCS_MODULE_TOUCH( EVCS_EVENT_WORK );
	}
}
void kernel_init(void *self)
{
	g_kernel_evts.evcb[ EVCS_EVENT_ROOT ] = (EVENTS_FUNCTION *)kernel_start;
}

void kernel_exit(void *self)
{
}

#ifdef __cplusplus
}
#endif
