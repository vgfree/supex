#pragma once

#include <ev.h>

#include "adopt_tasks/adopt_task.h"
#include "evmdl.h"

void alive_all_task_hit(struct adopt_task_node *task);

void alive_one_task_hit(struct adopt_task_node *task);

void alive_accept_cb(struct accept_module *p_mdl);

void alive_monitor_cb(struct monitor_module *p_mdl);

void alive_shell_cntl(const char *data);

void alive_signal_cb(struct signal_module *p_mdl);

void alive_update_cb(struct update_module *p_mdl);

void alive_stat_cb(struct reload_module *p_mdl);

void alive_upstream_cb(struct upstream_module *p_mdl);

void alive_dwstream_cb(struct dwstream_module *p_mdl);

