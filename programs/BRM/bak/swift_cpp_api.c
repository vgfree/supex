#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "swift_api.h"
#include "swift_cpp_api.h"
#include "apply_def.h"

#include "async_api.h"
#include "pool_api.h"
#define REDIS_ERR       -1
#define REDIS_OK        0

int swift_vms_init(void *W)
{
	return 0;
}

int swift_vms_call(void *W)
{
#if 0
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
	sniff_task.func = (SUPEX_TASK_CALLBACK)sniff_vms_call;	// fix to use xxxx;
	sniff_task.last = false;				// FIXME
	sniff_task.stamp = time(NULL);

	assert(p_rst->klen_array[0] < MAX_SNIFF_LABEL_LENGTH);

	memcpy(p_rmsg->label, p_buf + p_rst->key_offset[0], MIN(p_rst->klen_array[0], MAX_SNIFF_LABEL_LENGTH - 1));

	sniff_task.size = MIN((int)p_rst->vlen_array[0], MAX_SNIFF_FLOWS_LENGTH);

	if (sniff_task.size != p_rst->vlen_array[0]) {
		x_printf(E, "send size:%d real size:%d", sniff_task.size, p_rst->vlen_array[0]);
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
#endif	/* if 0 */

	return 0;
}

char TEST_HTTP_FORMAT[] = "POST /%s HTTP/1.1\r\n"
	"User-Agent: curl/7.33.0\r\n"
	"Host: %s:%d\r\n"
	"Content-Type: application/json; charset=utf-8\r\n"
	"Connection:%s\r\n"
	"Content-Length:%d\r\n"
	"Accept: */*\r\n\r\n%s";

void http_callback(struct async_ctx *ac, void *reply, void *data)
{
	x_printf(D, "-------------");

	if (reply) {
		struct net_cache *cache = reply;
		x_printf(D, "%s", cache->buf_addr);
	}
}

#define MAX_LEN_STRING          2048
#define ASYNC_LIBEV_THRESHOLD   10

int swift_vms_idle(void *W)
{
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)W;
	struct cnt_pool         *cpool = NULL;
	struct async_ctx        *ac = NULL;

	x_printf(D, "00000000000000000000000000");
	x_printf(I, "00000000000000000000000000");
	x_printf(W, "00000000000000000000000000");
	x_printf(E, "00000000000000000000000000");
	x_printf(S, "00000000000000000000000000");
	x_printf(F, "00000000000000000000000000");
#if 1
	ac = async_initial(p_swift_worker->loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, ASYNC_LIBEV_THRESHOLD);

	if (ac) {
		void    *sfd = (void *)(intptr_t)-1;
		int     rc = pool_api_gain(&cpool, "127.0.0.1", 6000, &sfd);

		if (rc) {
			//			pool_api_free ( cpool, &sfd );
			async_distory(ac);
			return -1;
		}

		/*data*/
		char    *proto;
		int     ok = cmd_to_proto(&proto, "SET key value");

		if (ok == REDIS_ERR) {
			pool_api_push(cpool, &sfd);
			async_distory(ac);
			return -1;
		}

		/*send*/
		async_command(ac, PROTO_TYPE_REDIS, (int)(intptr_t)sfd, cpool, NULL, NULL, proto, strlen(proto));
		free(proto);

		async_startup(ac);
		return 0;
	}
#endif	/* if 1 */

#if 1
	ac = async_initial(p_swift_worker->loop, QUEUE_TYPE_CORO, NULL, NULL, NULL, ASYNC_LIBEV_THRESHOLD);

	if (ac) {
		void    *sfd = (void *)(intptr_t)-1;
		int     rc = pool_api_gain(&cpool, "www.sina.com", 80, &sfd);

		if (rc) {
			//			pool_api_free ( cpool, &sfd );
			async_distory(ac);
			return -1;
		}

		/*data*/
		char buff[MAX_LEN_STRING] = {};
		snprintf(buff, MAX_LEN_STRING - 1, TEST_HTTP_FORMAT, "", "www.sina.com", 80, "Keep-Alive", 0, "");

		if (0) {
			pool_api_push(cpool, &sfd);
			async_distory(ac);
			return -1;
		}

		/*send*/
		async_command(ac, PROTO_TYPE_HTTP, (int)(intptr_t)sfd, cpool, (ASYNC_CALL_BACK)http_callback, NULL, buff, strlen(buff));

		async_startup(ac);
		return 0;
	}
#endif	/* if 1 */
	return 0;
}

