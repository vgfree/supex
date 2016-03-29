#include <string.h>
#include <stdlib.h>

#include "swift_api.h"
#include "swift_cpp_api.h"
#include "utils.h"

#include "xmq.h"
#include "xmq_msg.h"

extern enum pole_protype        g_iProType;
extern xmq_producer_t           *g_xmq_producer;

int swift_vms_call(void *W)
{
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)W;
	struct swift_task_node  *swift_task = &p_swift_worker->task;
	struct data_node        *p_node = get_pool_addr(swift_task->sfd);
	char                    *p_buf = p_node->recv.buf_addr;
	xmq_msg_t               *x_msg = NULL;

	switch (g_iProType & 0xFFFFF0)
	{
		case POLE_PROTYPE_HTTP:	// HTTP Protocal.
		{
			struct http_status *p_hst = &(p_node->http_info.hs);
			x_msg = xmq_msg_new_data((p_buf + p_hst->body_offset), p_hst->body_size);
			break;
		}

		case POLE_PROTYPE_REDIS:// REDIS Protocal.
		{
			struct redis_status *p_hst = &(p_node->redis_info.rs);
			x_msg = xmq_msg_new_data((p_buf + p_hst->val_offset[0]), (int)p_hst->vlen_array[0]);

			const char sndcnt[] = ":1\r\n";
			cache_add(&p_node->send, sndcnt, sizeof(sndcnt) - 1);
			break;
		}

		case POLE_PROTYPE_MTTP:	// MTTP Protocal.
		{
#ifdef _mttptest
			struct mttp_status *p_hst = &(p_node->mttp_info.mt);
			x_msg = xmq_msg_new_data((p_buf + p_hst->dosize + p_hst->headlen), p_hst->body_size);
#else
			x_printf(W, "MTTP Protocal doesn't used yet! should #define _mttp_test");
#endif
			break;
		}

		case POLE_PROTYPE_MFPTP:// MFPTP Protocal - doesn't implement yet!
		{
			//	struct http_status *p_hst = &(p_node->http_info.hs);
			//	x_msg = xmq_msg_new_data((p_buf + p_hst->body_offset), p_hst->body_size);
			x_printf(W, "MFPTP Protocal doesn't implement yet!");
			break;
		}

		default:
		{
			x_printf(W, "swift_vms_call: Invalid translate protocal.");
			return -1;
		}
	}

	// printf("Thread:%ld p_node:%p size:%d data:%s \n p_buf:%s \n=================\n",\
	//      //	(long)pthread_self(), p_node, (int)x_msg->len, x_msg->data, p_buf);

	int res = xmq_push_tail(g_xmq_producer, x_msg);
	xmq_msg_destroy(x_msg);

	if (res != 0) {
		x_printf(W, "swift_vms_call: xmq_push_tail(%s) %d bytes fail.", x_msg->data, (int)x_msg->len);
		return -1;
	}

	return 0;
}

