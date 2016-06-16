#pragma once

int swift_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int swift_vms_exec(void *user, union virtual_system **VMS, struct adopt_task_node *task);

