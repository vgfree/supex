#include <assert.h>

#include "major/swift_api.h"
#include "swift_cpp_api.h"

char *memdup(char *src, int size)
{
	char *new = calloc(1, size);
	assert(new);
	memcpy(new, src, size);
	return new;
}

int swift_vms_call(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	tlpool_t                *tlpool = user;
	int                     idx = tlpool_get_thread_index(tlpool);
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)&g_swift_worker_pthread[idx];
	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;


	int i;
	struct app_msg  send_msg = {0};
	send_msg.vector_size = p_rst->fields;
	for (i = 0; i < p_rst->fields; i++) {
		send_msg.vector[i].iov_base = memdup(p_buf + p_rst->field[i].offset, p_rst->field[i].len);
		send_msg.vector[i].iov_len = p_rst->field[i].len;
	}
	send_app_msg(&send_msg);


	const char sndcnt[] = ":1\r\n";
	cache_append(&p_node->mdl_send.cache, sndcnt, sizeof(sndcnt) - 1);

	return 0;
}
