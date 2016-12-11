#pragma once

#include <ev.h>
#include "async_obj.h"

struct async_evt
{
	struct async_obj        *obj;
	struct command_node     *cmd;
	struct ev_loop          *loop;
	int                     reading, writing;
	ev_io                   r_ev, w_ev;
};

void async_evt_add_recv(void **ev_impl);

void async_evt_del_recv(void **ev_impl);

void async_evt_add_send(void **ev_impl);

void async_evt_del_send(void **ev_impl);

void async_evt_cleanup(void **ev_impl);

void async_evt_produce(void **ev_impl);

void async_evt_init(struct async_obj *obj, struct command_node *cmd);

