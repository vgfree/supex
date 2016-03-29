#pragma once

#include "async_obj.h"
#include "async_cnt.h"
#include "async_evt.h"

struct async_ctx
{
	struct async_obj        obj;

	struct async_cnt        *links;

	void                    *data;
};

struct async_ctx        *async_initial(struct ev_loop *loop, enum queue_type qtype,
	LINK_CALL_BACK *usr_midway_stop_cb,
	RECY_CALL_BACK *usr_finish_work_cb,
	void *data, int peak);

void async_command(struct async_ctx *ctx,
	enum proto_type ptype,
	int sfd, struct cnt_pool *pool,
	ASYNC_CALL_BACK fcb, void *privdata,
	const char *data, size_t size);

void async_startup(struct async_ctx *ctx);

void async_distory(struct async_ctx *ctx);

