#pragma once
#include "evcs_events.h"

struct evcs_events g_y_evts = {};

void y_do(void *argv)
{
	printf("do y ...\n");
}

void y_init(void *self)
{
	g_y_evts.evcb[1] = (EVENTS_FUNCTION *)y_do;
}

void y_exit(void *self)
{}

