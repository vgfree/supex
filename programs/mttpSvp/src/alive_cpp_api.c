#include <string.h>
#include <stdlib.h>

#include "major/alive_api.h"
#include "alive_cpp_api.h"




int alive_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	printf("size %d data %s\n", task->package_size, task->package_data);

	return 0;
}

