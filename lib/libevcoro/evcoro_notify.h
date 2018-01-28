#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "libevcoro.h"
#include "list.h"

typedef bool (*EVCORO_NOTIFY_CHECK_FCB)(void *usr);

typedef struct evcoro_notify {
	struct list_head qlist;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} evcoro_notify_t;


void evcoro_notify_init(struct evcoro_notify *notify);

void evcoro_notify_free(struct evcoro_notify *notify);

void evcoro_notify_wait(struct evcoro_notify *notify, struct evcoro_scheduler *scheduler, EVCORO_NOTIFY_CHECK_FCB fcb, void *usr);

int evcoro_notify_wake(struct evcoro_notify *notify);
