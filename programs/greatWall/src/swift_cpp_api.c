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

int swift_vms_init(void *W)
{
	/*
	 *   SWIFT_WORKER_PTHREAD *p_swift_worker = (SWIFT_WORKER_PTHREAD *)W;
	 *   struct swift_task_node *swift_task = &p_swift_worker->task;
	 *
	 *   struct sniff_task_node sniff_task = {};
	 *   sniff_task.sfd = swift_task->sfd;
	 *   sniff_task.type = swift_task->type;
	 *   sniff_task.origin = swift_task->origin;
	 *   sniff_task.func = g_sniff_cfg_list.vmsys_init;
	 *   sniff_task.last = false;//FIXME
	 *   sniff_task.size = 0;
	 *
	 *   sniff_all_task_hit( (SNIFF_WORKER_PTHREAD *)p_swift_worker->mount, &sniff_task );
	 */
	return 0;
}

static int swift_vms_call_common(void *W, SUPEX_TASK_CALLBACK vms_cb)
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

	struct mount_info *mnt = (struct mount_info *)p_swift_worker->mount;

	while (mnt) {
		sniff_one_task_hit(mnt->list, &sniff_task);
		mnt->list = mnt->list->next;
		mnt = mnt->next;
	}

	const char sndcnt[] = ":1\r\n";
	cache_add(&p_node->send, sndcnt, sizeof(sndcnt) - 1);

	return 0;
}

int swift_vms_call_rpushx(void *W)
{
	return swift_vms_call_common(W, (SUPEX_TASK_CALLBACK)sniff_vms_call_rpushx);
}

int swift_vms_call_publish(void *W)
{
	return swift_vms_call_common(W, (SUPEX_TASK_CALLBACK)sniff_vms_call_publish);
}

#define BIT8_TASK_ORIGIN_IDLE 'd'
int swift_vms_idle(void *W)
{
	SWIFT_WORKER_PTHREAD *p_swift_worker = (SWIFT_WORKER_PTHREAD *)W;

	struct sniff_task_node sniff_task = {};

	sniff_task.sfd = 0;
	sniff_task.type = BIT8_TASK_TYPE_WHOLE;
	sniff_task.origin = BIT8_TASK_ORIGIN_IDLE;
	sniff_task.func = (SUPEX_TASK_CALLBACK)sniff_vms_idle;	// fix to use xxxx;
	sniff_task.last = false;				// FIXME
	sniff_task.size = 0;

	struct mount_info *mnt = (struct mount_info *)p_swift_worker->mount;

	while (mnt) {
		sniff_all_task_hit(mnt->list, &sniff_task);
		mnt = mnt->next;
	}

	return 0;
}

