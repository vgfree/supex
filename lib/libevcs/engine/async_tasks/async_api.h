#pragma once

#include "../pool_api/conn_xpool_api.h"
#include "async_ctx.h"

struct _async_cnt;
struct async_api;

typedef void (LINK_CALL_BACK)(const struct async_api *, void *data);
typedef void (RECY_CALL_BACK)(const struct async_api *, void *data);

struct async_api
{
	struct async_ctx        *ctx;
	bool                    auto_clean;

	LINK_CALL_BACK          *usr_midway_stop;
	RECY_CALL_BACK          *usr_finish_work;

	void                    *data;

	struct _async_cnt       *list;
};

struct async_api        *async_api_initial(struct ev_loop *loop, int peak, bool auto_clean, enum queue_type qtype, enum nexus_type ntype,
	LINK_CALL_BACK *usr_midway_stop_cb,
	RECY_CALL_BACK *usr_finish_work_cb,
	void *data);

struct command_node     *async_api_command(struct async_api *api, enum proto_type ptype, struct xpool *pool,
	const char *fops, const char *data1, size_t size1, const char *data2, size_t size2,
	ASYNC_CALL_BACK fcb, void *usr);

void async_api_startup(struct async_api *api);

void async_api_suspend(struct async_api *api);

void async_api_distory(struct async_api *api);

