#pragma once

#include "async_tasks/async_api.h"
#include "evcoro_scheduler.h"

struct async_evtasker
{
	struct async_api        *cmdt;/*使用连接池的方式*/
	size_t			cmds;

	struct async_ctx	*exet;/*使用描述符的方式*/
	size_t			exes;

	struct evcoro_scheduler *scheduler;
	struct ev_coro          *repair;
};

struct async_evtasker   *evtask_initial(struct evcoro_scheduler *scheduler, int peak, enum queue_type qtype, enum nexus_type ntype);
/*
 * 当协议属于为PROTO_ATTR_VARY,则设置的设置的read位参数data和size均无效。
 */
struct command_node     *evtask_dispose(struct async_evtasker *sevt, enum proto_type ptype, struct xpool *pool,
		const char *fops, const char *data1, size_t size1, const char *data2, size_t size2);

struct command_node     *evtask_execute(struct async_evtasker *sevt, enum proto_type ptype, int fd,
		const char *fops, const char *data1, size_t size1, const char *data2, size_t size2);

void evtask_install(struct async_evtasker *sevt);

void evtask_offload(struct async_evtasker *sevt);

void evtask_distory(struct async_evtasker *sevt);

void evtask_startup(struct async_evtasker *sevt);

