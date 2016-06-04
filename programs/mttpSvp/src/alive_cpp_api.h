#pragma once
#include "base/utils.h"


int alive_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task);
int alive_vms_online(void *user, union virtual_system **VMS, struct adopt_task_node *task);
int alive_vms_offline(void *user, union virtual_system **VMS, struct adopt_task_node *task);

