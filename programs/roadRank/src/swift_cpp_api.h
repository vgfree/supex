#pragma once

int swift_vms_init(void *W);

//int swift_vms_call(void *W);
int swift_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task);

