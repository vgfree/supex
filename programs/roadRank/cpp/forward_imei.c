#include "forward_imei.h"
#include "pool_api.h"

extern struct rr_cfg_file g_rr_cfg_file;

bool forward_imei_check(long long imei)
{
	if ((imei <= 0) || (g_rr_cfg_file.imei_count <= 0)) {
		return false;
	}

	int i;

	for (i = 0; i < g_rr_cfg_file.imei_count; i++) {
		if (g_rr_cfg_file.imei_buff[i] == imei) {
			return true;
		}
	}

	return false;
}

bool forward_data(const char *data, struct ev_loop *loop, char *host, int port)
{
	if (!data) {
		return false;
	}

	struct async_ctx        *ac = NULL;
	struct cnt_pool         *cpool = NULL;

	char http_data[20480] = { '\0' };

	char HTTP_FORMAT[] = "POST /%s HTTP/1.1\r\n"
		"User-Agent: curl/7.33.0\r\n"
		"Host: %s:%d\r\n"
		"Content-Type: application/json; charset=utf-8\r\n"
		"Connection:%s\r\n"
		"Content-Length:%d\r\n"
		"Accept: */*\r\n\r\n%s";
	snprintf(http_data, 20480 - 1, HTTP_FORMAT, "publicentry", host, port, "Keep-Alive", strlen(data), data);

	ac = async_initial(loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, 1);

	if (ac) {
		void    *sfd = (void *)(intptr_t)-1;
		int     rc = conn_xpool_gain(&cpool, host, port, &sfd);

		if (rc) {
			async_distory(ac);
			return -1;
		}

		async_command(ac, PROTO_TYPE_HTTP, (intptr_t)(void *)sfd, cpool, NULL, NULL, http_data, strlen(http_data));
		async_startup(ac);
	}

	return true;
}

