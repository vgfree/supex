#include <string.h>
#include <stdlib.h>

#include "base/utils.h"
#include "major/smart_api.h"
#include "smart_evcoro_cpp_api.h"
#include "entry.h"
#include "pmr_locate.h"
#include "pmr_qlocate.h"
#include "pmr_mlocate.h"

int api_hmget(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	struct data_node        *p_node = get_pool_data(task->sfd);

	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	// printf("%d %s\n", p_rst->flen_array[0], p_buf + p_rst->fld_offset[0]);
	if (0 == strncmp(p_buf + p_rst->field[0].offset, "LOCATE", MAX(p_rst->field[0].len, 6))) {
		return entry_cmd_locate(p_node);
	} else if (0 == strncmp(p_buf + p_rst->field[0].offset, "QLOCATE", MAX(p_rst->field[0].len, 7))) {
		return entry_cmd_qlocate(p_node);
	} else if (0 == strncmp(p_buf + p_rst->field[0].offset, "MLOCATE", MAX(p_rst->field[0].len, 7))) {
		return entry_cmd_mlocate(p_node);
	}


	cache_append(&p_node->mdl_send.cache, OPT_MULTI_BULK_FALSE, strlen(OPT_MULTI_BULK_FALSE));
	return 0;
}

int api_hgetall(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	struct data_node        *p_node = get_pool_data(task->sfd);

	cache_append(&p_node->mdl_send.cache, OPT_MULTI_BULK_FALSE, strlen(OPT_MULTI_BULK_FALSE));
	return -1;
}

