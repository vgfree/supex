#pragma once

#include "lj_sniff_util.h"

int sniff_vms_init(void *user, union virtual_system **VMS, struct sniff_task_node *task);

int sniff_vms_call_ext(void *user, union virtual_system **VMS, struct sniff_task_node *task);

int sniff_vms_sync_ext(void *user, union virtual_system **VMS, struct sniff_task_node *task);

int sniff_vms_gain_ext(void *user, union virtual_system **VMS, struct sniff_task_node *task);

