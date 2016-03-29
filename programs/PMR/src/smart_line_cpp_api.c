#include <string.h>
#include <stdlib.h>

#include "smart_line_cpp_api.h"
#include "utils.h"
#include "smart_api.h"
#include "entry.h"
#include "pmr_locate.h"
#include "pmr_qlocate.h"
#include "pmr_mlocate.h"

int api_hmget(void *user, void *task)
{
//	SMART_WORKER_PTHREAD    *p_smart_worker = (SMART_WORKER_PTHREAD *)user;
	struct smart_task_node  *p_task = (struct smart_task_node *)task;
	struct data_node        *p_node = get_pool_addr(p_task->sfd);

	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	// printf("%d %s\n", p_rst->flen_array[0], p_buf + p_rst->fld_offset[0]);
	if (0 == strncmp(p_buf + p_rst->fld_offset[0], "LOCATE", MAX(p_rst->flen_array[0], 6))) {
		return entry_cmd_locate(p_node);
	} else if (0 == strncmp(p_buf + p_rst->fld_offset[0], "QLOCATE", MAX(p_rst->flen_array[0], 7))) {
		return entry_cmd_qlocate(p_node);
	} else if (0 == strncmp(p_buf + p_rst->fld_offset[0], "MLOCATE", MAX(p_rst->flen_array[0], 7))) {
		return entry_cmd_mlocate(p_node);
	}


	cache_add(&p_node->send, OPT_MULTI_BULK_FALSE, strlen(OPT_MULTI_BULK_FALSE));
	return 0;
}

int api_hgetall(void *user, void *task)
{
	struct smart_task_node  *p_task = (struct smart_task_node *)task;
	struct data_node        *p_node = get_pool_addr(p_task->sfd);

	cache_add(&p_node->send, OPT_MULTI_BULK_FALSE, strlen(OPT_MULTI_BULK_FALSE));
	return -1;
}

