#include <string.h>
#include <stdlib.h>

#include "major/swift_api.h"
#include "swift_cpp_api.h"
#include "base/utils.h"

#include "xmq.h"
#include "xmq_msg.h"

extern xmq_producer_t           *g_xmq_p1;
extern struct swift_cfg_list    g_swift_cfg_list;

int swift_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	xmq_msg_t               *x_msg = NULL;

	switch (g_swift_cfg_list.file_info.ptype)
	{
		case USE_HTTP_PROTO:
		{
			struct http_status *p_hst = &(p_node->mdl_recv.parse.http_info.hs);
			x_msg = xmq_msg_new_data((p_buf + p_hst->body_offset), p_hst->body_size);
			break;
		}

		case USE_REDIS_PROTO:
		{
			struct redis_status *p_rst = &(p_node->mdl_recv.parse.redis_info.rs);
			x_msg = xmq_msg_new_data((p_buf + p_rst->field[1].offset), (int)p_rst->field[1].len);

			const char sndcnt[] = ":1\r\n";
			cache_append(&p_node->mdl_send.cache, sndcnt, sizeof(sndcnt) - 1);
			break;
		}

		case USE_MTTP_PROTO:	// fixme
		{
#ifdef _mttptest
			struct mttp_status *p_hst = &(p_node->mttp_info.mt);
			x_msg = xmq_msg_new_data((p_buf + p_hst->dosize + p_hst->headlen), p_hst->body_size);
#else
			x_printf(W, "MTTP Protocal doesn't used yet! should #define _mttp_test");
#endif
			break;
		}

#if 0
		case USE_MFPTP_PROTO:	// MFPTP Protocal - doesn't implement yet!
		{
			//	struct http_status *p_hst = &(p_node->http_info.hs);
			//	x_msg = xmq_msg_new_data((p_buf + p_hst->body_offset), p_hst->body_size);
			x_printf(W, "MFPTP Protocal doesn't implement yet!");
			break;
		}
#endif
		default:
		{
			x_printf(W, "swift_vms_call: Invalid translate protocal.");
			return -1;
		}
	}

	// printf("Thread:%ld p_node:%p size:%d data:%s \n p_buf:%s \n=================\n",\
	//      //	(long)pthread_self(), p_node, (int)x_msg->len, x_msg->data, p_buf);

	int res = xmq_push_tail(g_xmq_p1, x_msg);

	if (res != 0) {
		x_printf(W, "swift_vms_call: xmq_push_tail(%s) %d bytes fail.", x_msg->data, (int)x_msg->len);
	}

	xmq_msg_destroy(x_msg);

	return res;
}

