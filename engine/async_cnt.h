#pragma once

#include "cnt_pool.h"

struct async_cnt
{
	intptr_t                sfd;
	int                     idx;
	struct cnt_pool         *pool;
	struct async_cnt        *next;
};

void async_cnt_free(struct async_cnt **head, int efd);

void async_cnt_add(struct async_cnt **head, int sfd, struct cnt_pool *pool);

