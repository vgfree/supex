#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "base/utils.h"
#include "adopt_tasks/adopt_task.h"

int smart_vms_exit(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int smart_vms_cntl(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int smart_vms_rfsh(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int smart_vms_gain(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int smart_vms_sync(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int smart_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int smart_vms_exec(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int smart_vms_monitor(void *user, union virtual_system **VMS, struct adopt_task_node *task);

