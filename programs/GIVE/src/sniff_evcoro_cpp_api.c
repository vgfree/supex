#include <assert.h>
#include <unistd.h>

#include "gv_common.h"
#include "minor/sniff_api.h"
#include "sniff_evcoro_cpp_api.h"
#include "rr_cfg.h"
#include "tcp_api/tcp_api.h"
#include "async_tasks/async_api.h"
#include "pool_api/conn_xpool_api.h"
#include "base/utils.h"
#include "redis_api/redis_status.h"
#include "count.h"
#include "rr_cfg.h"

#define MTP_VERSION     0x10
#define MTP_FORMAT      0x20
int send_protocbuff(void *sfd, int size, char *result);

#define REDIS_ERR       -1
#define REDIS_OK        0
extern struct rr_cfg_file g_rr_cfg_file;

extern int cjson_topb(const char *data, char **result, data_count_t *dt);

extern int gzpb_to_str(char *value, int data_len);

extern kv_handler_t *count_handler;



int sniff_vms_call(void *user, union virtual_system **VMS, struct sniff_task_node *task)
{
	struct sniff_task_node  *p_task = (struct sniff_task_node *)task;
	time_t                  delay = time(NULL) - p_task->stamp;

	x_printf(D, "task <shift> %d\t<come> %ld\t<delay> %ld",
		p_task->base.shift, p_task->stamp, delay);

	x_printf(I, "%s\n", p_task->data);

	char            *result = NULL;
	data_count_t    dt = { 0, 0, 0, 0 };
	int             size = cjson_topb(p_task->data, &result, &dt);

	if ((size == GV_FILTER) || (size == GV_EMPTY)) {// lon and lat is not in valid city
		x_printf(W, "filter or empty: %d\n", size);
		return GV_ERR;
	}

	if (!result || (size < 0)) {
		x_printf(E, "cjson to protocol buffer failed !\n");
		return GV_ERR;
	}

	struct xpool *cpool = NULL;
	void            *sfd = (void *)-1;
	int             rc = conn_xpool_gain(&cpool, g_rr_cfg_file.gaode_host, g_rr_cfg_file.gaode_port, &sfd);

	if (rc) {
		//		poll_api_free ( cpool, &sfd );
		free(result);
		return GV_ERR;
	}

	if (send_protocbuff(sfd, size, result) < 0) {
		conn_xpool_free(cpool, &sfd);
		free(result);
		return GV_ERR;
	}

	int mem = city_data_count(count_handler, &dt);

	if (mem < 0) {
		x_printf(E, "count failed .\n");
	}

	conn_xpool_push(cpool, &sfd);
	free(result);
	return GV_OK;
}

int send_protocbuff(void *sfd, int size, char *result)
{
	unsigned int net_size = htonl((unsigned int)size);

	int     len = 0;
	char    headbuf[6];

	headbuf[0] = MTP_VERSION;
	headbuf[1] = MTP_FORMAT;
	memcpy(headbuf + 2, (char *)&net_size, 4);
#ifndef _GAODE
	len = 6;
#else
	len = 4;
#endif
	int out = 0;

	while (out < len) {
		int now = send(sfd, headbuf + out + 6 - len, len - out, 0);
		x_printf(I, "send datahead len:%d\n", now);

		if (now >= 0) {
			out += now;
		} else {
			x_printf(E, "send head error:%d-->%s.\n", errno, strerror(errno));
			return GV_ERR;
		}
	}

	x_printf(I, "send headsize:%d\n", out);
	out = 0;

	while (out < size) {
		int now = send(sfd, result + out, size - out, 0);
		x_printf(I, "send size:%d\n", now);

		if (now >= 0) {
			out += now;
		} else {
			x_printf(E, "send error:%d-->%s!\n", errno, strerror(errno));
			return GV_ERR;
		}
	}

	x_printf(I, "send bodysize:%d--size is %d\n", out, size);
	return GV_OK;
}

