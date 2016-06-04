#pragma once
#include "evcs_events.h"

struct evcs_events g_x_evts = {};

void x_do(void *argv)
{
	printf("do x ...\n");
}

void x_init(void *self)
{
	g_x_evts.evcb[0] = (EVENTS_FUNCTION *)x_do;
}

void x_exit(void *self)
{}

