#pragma once

#include "async_tasks/async_api.h"
#include "evcoro_scheduler.h"

struct async_evtasker
{
	struct async_api        *api;
	struct evcoro_scheduler *scheduler;
	struct ev_coro          *repair;
};

struct async_evtasker   *evtask_initial(struct evcoro_scheduler *scheduler, int peak, enum queue_type qtype, enum nexus_type ntype);

struct command_node     *evtask_command(struct async_evtasker *sevt, enum proto_type ptype, struct xpool *pool, const char *data, size_t size);

void evtask_install(struct async_evtasker *sevt);

void evtask_offload(struct async_evtasker *sevt);

void evtask_distory(struct async_evtasker *sevt);

void evtask_startup(struct evcoro_scheduler *scheduler);

