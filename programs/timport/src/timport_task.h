#pragma once

#include "ev.h"

#include "redis_parse.h"

#include "timport_cfg.h"

#define MAX_KEY_SIZE 128

typedef struct timport_task
{
	struct ev_loop          *loop;
	timport_key_t           *tkey;
	struct redis_link       *rds_link;
	char                    *ftime;
	char                    *param;
	int                     param_len;
	char                    key[MAX_KEY_SIZE];
	struct redis_reply      *reply;
	int                     total_result_count;
	int                     finish_result_count;
	int                     total_task_count;
	int                     finish_task_count;
	int                     next_task_count;
	int                     step;
	struct timport_task     *child;
	struct timport_task     *slibing;
	struct timport_task     *parent;
} timport_task_t;

void timport_task_init(void);

void timport_task_exit(void);

void timport_task_check(void *user);

