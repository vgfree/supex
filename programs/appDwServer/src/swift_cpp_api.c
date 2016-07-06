#include <assert.h>

#include "major/swift_api.h"
#include "swift_cpp_api.h"

int swift_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	tlpool_t                *tlpool = user;
	int                     idx = tlpool_get_thread_index(tlpool);
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[idx];
	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	assert(p_rst->field[0].len < MAX_SNIFF_LABEL_LENGTH);

	memcpy(p_rmsg->label, p_buf + p_rst->field[0].offset, MIN(p_rst->field[0].len, MAX_SNIFF_LABEL_LENGTH - 1));

	sniff_task.size = MIN((int)p_rst->field[1].len, MAX_SNIFF_FLOWS_LENGTH);

	if (sniff_task.size != p_rst->field[1].len) {
		x_printf(E, "send size:%d real size:%zd", sniff_task.size, p_rst->field[1].len);
	}

	memcpy(p_rmsg->flows, p_buf + p_rst->field[1].offset, sniff_task.size);


	const char sndcnt[] = ":1\r\n";
	cache_append(&p_node->mdl_send.cache, sndcnt, sizeof(sndcnt) - 1);

	return 0;
}
