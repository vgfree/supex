#pragma once

#include <ev.h>

#include "adopt_tasks/adopt_task.h"
#include "evmdl.h"

void swift_all_task_hit(struct adopt_task_node *task, bool synch, int mark);

void swift_one_task_hit(struct adopt_task_node *task, bool synch, int mark, unsigned int step);

void swift_accept_cb(struct accept_module *p_mdl);

void swift_monitor_cb(struct monitor_module *p_mdl);

void swift_shell_cntl(const char *data);

void swift_signal_cb(struct signal_module *p_mdl);

void swift_update_cb(struct update_module *p_mdl);

void swift_stat_cb(struct reload_module *p_mdl);

void swift_recv_cb(struct irecv_module *p_mdl);

void swift_send_cb(struct osend_module *p_mdl);

void swift_timeout_cb(struct iotime_module *p_mdl);

void swift_fetch_cb(struct pipe_module *p_mdl);

