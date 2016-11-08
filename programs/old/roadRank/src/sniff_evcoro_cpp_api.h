#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int sniff_vms_init(void *user, void *task);

//int sniff_vms_call(void *user, void *task);
int sniff_vms_call(void *user, union virtual_system **VMS, struct sniff_task_node *task);

