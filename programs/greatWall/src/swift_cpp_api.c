#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "swift_api.h"
#include "sniff_api.h"
#include "swift_cpp_api.h"
#include "apply_def.h"

#ifdef OPEN_SCCO
  #include "sniff_scco_lua_api.h"
#endif
#ifdef OPEN_LINE
  #include "sniff_line_cpp_api.h"
#endif
#ifdef OPEN_EVUV
  #include "sniff_evuv_cpp_api.h"
#endif

extern struct sniff_cfg_list g_sniff_cfg_list;

static int swift_vms_call_common(void *W, SNIFF_VMS_FCB vms_cb)
{
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)W;
	struct swift_task_node  *swift_task = &p_swift_worker->task;
	struct data_node        *p_node = get_pool_addr(swift_task->sfd);
	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	struct sniff_task_node  sniff_task = {};
	struct route_msg_data   *p_rmsg = (struct route_msg_data *)sniff_task.data;

	sniff_task.sfd = swift_task->sfd;
	sniff_task.type = swift_task->type;
	sniff_task.origin = swift_task->origin;
	sniff_task.func = vms_cb;	// fix to use xxxx;
	sniff_task.last = false;	// FIXME
	sniff_task.stamp = time(NULL);

	assert(p_rst->klen_array[0] < MAX_SNIFF_LABEL_LENGTH);

	memcpy(p_rmsg->label, p_buf + p_rst->key_offset[0], MIN(p_rst->klen_array[0], MAX_SNIFF_LABEL_LENGTH - 1));

	sniff_task.size = MIN((int)p_rst->vlen_array[0], MAX_SNIFF_FLOWS_LENGTH);

	if (sniff_task.size != p_rst->vlen_array[0]) {
		x_printf(E, "send size:%d real size:%zd", sniff_task.size, p_rst->vlen_array[0]);
	}

	memcpy(p_rmsg->flows, p_buf + p_rst->val_offset[0], sniff_task.size);

	/*如果有多通道，复制发给所以通道*/
	SNIFF_WORKER_PTHREAD *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)p_swift_worker->mount;
	sniff_one_task_hit(p_sniff_worker, &sniff_task);
	p_swift_worker->mount = p_sniff_worker->next;

	const char sndcnt[] = ":1\r\n";
	cache_add(&p_node->send, sndcnt, sizeof(sndcnt) - 1);

	return 0;
}

int swift_vms_call_rlpushx(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	return swift_vms_call_common(user, (SNIFF_VMS_FCB)sniff_vms_call_rlpushx);
}

int swift_vms_call_publish(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	return swift_vms_call_common(user, (SNIFF_VMS_FCB)sniff_vms_call_publish);
}

