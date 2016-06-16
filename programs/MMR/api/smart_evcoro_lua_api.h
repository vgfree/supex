#pragma once

#include "lj_smart_util.h"

int smart_vms_init(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int smart_vms_call_ext(void *user, union virtual_system **VMS, struct adopt_task_node *task);

