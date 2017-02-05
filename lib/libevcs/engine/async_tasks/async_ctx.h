#pragma once

#include "async_obj.h"
#include "async_evt.h"

struct async_ctx;
/* Connection callback prototypes */
typedef void (_LINK_CALL_BACK)(const struct async_ctx *, void *data);
typedef void (_RECY_CALL_BACK)(const struct async_ctx *, void *data);

struct async_ctx
{
	struct async_obj        obj;
	bool                    auto_clean;

	_LINK_CALL_BACK         *_midway_stop;
	_RECY_CALL_BACK         *_finish_work;

	void                    *data;
};

/***************************
* 有序VS无序 * 集群VS独立 *
***************************/
struct async_ctx        *async_ctx_initial(struct ev_loop *loop, int peak, bool auto_clean, enum queue_type qtype, enum nexus_type ntype,
	_LINK_CALL_BACK *midway_stop_cb,
	_RECY_CALL_BACK *finish_work_cb,
	void *data);

struct command_node     *async_ctx_command(struct async_ctx *ctx, enum proto_type ptype, int sfd,
	const char *fops, const char *data1, size_t size1, const char *data2, size_t size2,
	ASYNC_CALL_BACK fcb, void *usr);

void async_ctx_startup(struct async_ctx *ctx);

void async_ctx_suspend(struct async_ctx *ctx);

void async_ctx_distory(struct async_ctx *ctx);

