#include <string.h>
#include <stdlib.h>

#include "base/utils.h"
#include "major/smart_api.h"
#include "adopt_tasks/adopt_task.h"
#include "smart_evcoro_cpp_api.h"
#include "entry.h"

int api_hmget(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	struct data_node        *p_node = get_pool_data(task->sfd);

	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	// printf("%d %s\n", p_rst->flen_array[0], p_buf + p_rst->fld_offset[0]);
	if (!strncmp(p_buf + p_rst->field[0].offset, "ERBR", MAX(p_rst->field[0].len, 4))) {
		return entry_cmd_erbr(p_node,
			       strtoull(p_buf + p_rst->field[0].offset, NULL, 10));
	}

	if (!strncmp(p_buf + p_rst->field[0].offset, "IRBR", MAX(p_rst->field[0].len, 4))) {
		return entry_cmd_irbr(p_node,
			       strtoull(p_buf + p_rst->field[0].offset, NULL, 10));
	}

	if (!strncmp(p_buf + p_rst->field[0].offset, "EROF", MAX(p_rst->field[0].len, 4))) {
		return entry_cmd_erof(p_node,
			       strtoull(p_buf + p_rst->field[0].offset, NULL, 10));
	}

	if (!strncmp(p_buf + p_rst->field[0].offset, "IROF", MAX(p_rst->field[0].len, 4))) {
		return entry_cmd_irof(p_node,
			       strtoull(p_buf + p_rst->field[0].offset, NULL, 10));
	}

	if (!strncmp(p_buf + p_rst->field[0].offset, "ENBR", MAX(p_rst->field[0].len, 4))) {
		return entry_cmd_enbr(p_node,
			       strtoull(p_buf + p_rst->field[0].offset, NULL, 10));
	}

	if (!strncmp(p_buf + p_rst->field[0].offset, "FNBR", MAX(p_rst->field[0].len, 4))) {
		return entry_cmd_fnbr(p_node,
			       strtoull(p_buf + p_rst->field[0].offset, NULL, 10));
	}

	cache_append(&p_node->mdl_send.cache, OPT_MULTI_BULK_FALSE, strlen(OPT_MULTI_BULK_FALSE));
	return 0;
}

int api_hgetall(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	struct data_node        *p_node = get_pool_data(task->sfd);

	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if (!strncmp(p_buf + p_rst->field[0].offset, "BEELINE", 7)) {
		return entry_cmd_rlbr(p_node, 0);
	}

	/*
	 *   if ( !strncmp(p_buf + p_rst->field[0].offse, "SCALLOP", 7) ){
	 *        return entry_cmd_rlbr( p_node, 0 );
	 *   }
	 */

	cache_append(&p_node->mdl_send.cache, OPT_OK, strlen(OPT_OK));
	return 0;
}

