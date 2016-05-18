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
	struct async_evt        *next;
};

void async_evt_recv(struct ev_loop *loop, ev_io *watcher, int revents);

void async_evt_send(struct ev_loop *loop, ev_io *watcher, int revents);

void async_evt_add_recv(void *privdata);

void async_evt_del_recv(void *privdata);

void async_evt_add_send(void *privdata);

void async_evt_del_send(void *privdata);

void async_evt_cleanup(void *privdata);

void async_evt_produce(void *privdata);

void async_evt_init(struct async_obj *obj, struct command_node *cmd, int sfd);

