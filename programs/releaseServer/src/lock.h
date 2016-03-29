#pragma once

typedef struct _WAIT
{
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	int             count;
} WAIT;

typedef struct _SYNC_WAIT
{
	WAIT    *wait;
	void    *data;
} SYNC_WAIT;

