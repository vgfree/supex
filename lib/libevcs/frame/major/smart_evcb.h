#pragma once

#include <ev.h>

#include "engine/adopt_tasks/adopt_task.h"
#include "engine/proto_comm.h"
#include "engine/evmdl.h"

void smart_dispatch_task(int type, int sfd);

void smart_all_task_hit(struct adopt_task_node *task, bool synch, int mark);

void smart_one_task_hit(struct adopt_task_node *task, bool synch, int mark, unsigned int step);

void smart_accept_cb(struct accept_module *p_mdl);

/*
 * 名  称:smart_monitor_cb
 * 功  能:周期循环事件的回调函数
 * 参  数:loop 事件循环，　w　周期循环事件，　
 * 返回值:
 * 修  改:添加新函数,程少远 2015/05/12
 */
void smart_monitor_cb(struct monitor_module *p_mdl);

void smart_shell_cntl(const char *data);

void smart_update_cb(struct update_module *p_mdl);

void smart_recv_cb(struct irecv_module *p_mdl);

void smart_send_cb(struct osend_module *p_mdl);

void smart_timeout_cb(struct iotime_module *p_mdl);

void smart_fetch_recv_cb(struct pipe_module *p_mdl);

void smart_fetch_send_cb(struct pipe_module *p_mdl);

