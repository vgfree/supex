#include <stdio.h>
#include "evcs_module.h"
#include "evcs_events.h"
#include "evcs_kernel.h"
#include "spx_evcs_module.h"

EVCS_MODULE_SETUP(kernel, kernel_init, kernel_exit, &g_kernel_evts);
EVCS_MODULE_SETUP(evcs, evcs_init, evcs_exit, &g_evcs_evts);

void main(void)
{
	/*load module*/
	EVCS_MODULE_MOUNT(kernel);
	EVCS_MODULE_ENTRY(kernel, true);

	struct evcs_argv_settings sets = {
		.num    = 5
		, .tsz  = 1024
		, .data = NULL
		, .task_lookup= NULL
		, .task_handle= NULL
	};
	EVCS_MODULE_CARRY(evcs, &sets);
	EVCS_MODULE_MOUNT(evcs);
	EVCS_MODULE_ENTRY(evcs, true);

	/*work events*/
	EVCS_MODULE_START();
}

