#include <string.h>
#include <stdlib.h>

#include "base/utils.h"
#include "major/swift_api.h"
#include "minor/sniff_api.h"
#include "swift_cpp_api.h"
#include "apply_def.h"

#include "sniff_evcoro_cpp_api.h"

extern struct sniff_cfg_list g_sniff_cfg_list;

static int swift_vms_call_common(void *user, struct adopt_task_node *task, SNIFF_VMS_FCB vms_cb)
{
	tlpool_t	*tlpool = user;
	int idx = tlpool_get_thread_index(tlpool);
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[idx];
	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	struct sniff_task_node  sniff_task = {};
	struct route_msg_data   *p_rmsg = (struct route_msg_data *)sniff_task.data;

	sniff_task.sfd = task->sfd;
	sniff_task.type = task->type;
	sniff_task.origin = task->origin;
	sniff_task.func = vms_cb;	// fix to use xxxx;
	sniff_task.last = false;	// FIXME
	sniff_task.stamp = time(NULL);

	assert(p_rst->field[0].len < MAX_SNIFF_LABEL_LENGTH);

	memcpy(p_rmsg->label, p_buf + p_rst->field[0].offset, MIN(p_rst->field[0].len, MAX_SNIFF_LABEL_LENGTH - 1));

	sniff_task.size = MIN((int)p_rst->field[1].len, MAX_SNIFF_FLOWS_LENGTH);

	if (sniff_task.size != p_rst->field[1].len) {
		x_printf(E, "send size:%d real size:%zd", sniff_task.size, p_rst->field[1].len);
	}

	memcpy(p_rmsg->flows, p_buf + p_rst->field[1].offset, sniff_task.size);

	/*如果有多通道，复制发给所以通道*/
	g_sniff_cfg_list.task_report(p_swift_worker->mount, &sniff_task);

	const char sndcnt[] = ":1\r\n";
	cache_append(&p_node->mdl_send.cache, sndcnt, sizeof(sndcnt) - 1);

	return 0;
}

int swift_vms_call_rlpushx(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	return swift_vms_call_common(user, task, (SNIFF_VMS_FCB)sniff_vms_call_rlpushx);
}

int swift_vms_call_publish(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	return swift_vms_call_common(user, task, (SNIFF_VMS_FCB)sniff_vms_call_publish);
}

