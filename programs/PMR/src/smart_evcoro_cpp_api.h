#pragma once

#include "adopt_tasks/adopt_task.h"

int api_hmget(void *user, union virtual_system **VMS, struct adopt_task_node *task);

int api_hgetall(void *user, union virtual_system **VMS, struct adopt_task_node *task);

