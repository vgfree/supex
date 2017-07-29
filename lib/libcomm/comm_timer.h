#pragma once
#include <stdint.h>
#include <stdbool.h>

struct comm_timer
{
	int tmfd;
	bool active;
};

int commtimer_init(struct comm_timer *timer);

/*delay ms*/
int commtimer_wait(struct comm_timer *timer, long delay);

int commtimer_stop(struct comm_timer *timer);

uint64_t commtimer_grab(struct comm_timer *timer);

int commtimer_free(struct comm_timer *timer);
