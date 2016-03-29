#pragma once

#include <pthread.h>
#include "xlist.h"
#include "netmod.h"

typedef struct pipe_struct
{
	xlist_t         head;
	unsigned int    count;

	unsigned int    wait;
	pthread_mutex_t lock;
	pthread_cond_t  cond;
} pipe_t;

#define pipe_entrys(p) ((p)->count)

typedef struct pipe_entry_struct
{
	xlist_t  list;
	event_t *event;
} pipe_entry_t;

extern void pipe_init(pipe_t *pipe);

extern void pipe_release(pipe_t *pipe);

extern event_t *pipe_pull(pipe_t *pipe, int millis);

extern event_t *pipe_pull_nolock(pipe_t *pipe);

extern int pipe_push(pipe_t *pipe, event_t *event);

extern int pipe_push_nolock(pipe_t *pipe, event_t *event);

extern int pipe_push_back(pipe_t *pipe, event_t *event);

extern int pipe_push_back_nolock(pipe_t *pipe, event_t *event);

