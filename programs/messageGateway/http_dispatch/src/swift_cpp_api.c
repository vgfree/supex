#include <string.h>
#include <stdlib.h>

#include "swift_api.h"
#include "swift_cpp_api.h"
#include "utils.h"

char    *buff[1024] = {};
int     size = 0;

int swift_vms_call(void *W)
{
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)W;
	struct swift_task_node  *swift_task = &p_swift_worker->task;

	struct data_node        *p_node = get_pool_addr(swift_task->sfd);
	char                    *p_buf = p_node->recv.buf_addr;
	struct http_status      *p_hst = &p_node->http_info.hs;

	if (p_hst->body_size == 0) {
		return -1;
	}

	// get_cache_data(swift_task->sfd, buff, &size);
	// printf("%s\n", buff);

	return 0;
}

