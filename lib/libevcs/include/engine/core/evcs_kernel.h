#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include "evcs_module.h"
#include "evcs_events.h"

extern struct evcs_events g_kernel_evts = {};

static void kernel_start(void *argv)
{
	struct timeval  sta = {};
	struct timeval  end = {};
	long            diffms = 0;

	EVCS_MODULE_TOUCH(EVCS_EVENT_INIT);

	while (1) {
		gettimeofday(&sta, NULL);

		EVCS_MODULE_TOUCH(EVCS_EVENT_WORK);

		gettimeofday(&end, NULL);

		diffms = (end.tv_sec - sta.tv_sec) * 1000;
		diffms += (end.tv_usec - sta.tv_usec) / 1000;
	//	printf("use time %ld\n", diffms);

		if (diffms < 2) {
			usleep(500);
		}
	}
}

void kernel_init(void *self)
{
	g_kernel_evts.evcb[EVCS_EVENT_ROOT] = (EVENTS_FUNCTION *)kernel_start;
}

void kernel_exit(void *self)
{}

#ifdef __cplusplus
}
#endif

