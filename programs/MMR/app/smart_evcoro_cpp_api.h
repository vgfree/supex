#pragma once

#include "lj_smart_util.h"

int smart_vms_gain_ext(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int smart_vms_sync_ext(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int smart_vms_monitor_ext(void *user, union virtual_system **VMS, struct adopt_task_node *task);
