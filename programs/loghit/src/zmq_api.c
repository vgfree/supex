#include "zmq_api.h"
/*==============================use zmq=====================================*/
static void     *g_ctx = NULL;
static void     *g_skt = NULL;

void zmqskt_init(char *host, int port)
{
	// 创建一个新的zmq上下文
	g_ctx = zmq_ctx_new();
	// 创建套接字及消息队列类型
	g_skt = zmq_socket(g_ctx, ZMQ_PUSH);

	char addr[64] = {};
	snprintf(addr, sizeof(addr), "tcp://%s:%d", host, port);
	// 与远程zmq端点建立连接
	int ok = zmq_connect(g_skt, addr);

	if (ok == -1) {
		printf("%s\n", zmq_strerror(zmq_errno()));
	}

	assert(ok == 0);
}

void zmqskt_exit(void)
{
	zmq_close(g_skt);
	zmq_ctx_destroy(g_ctx);
}

void zmqmsg_start(struct skt_device *devc, void *skt, size_t num)
{
	assert(devc);
	memset(devc, 0, sizeof(struct skt_device));
	devc->skt = skt ? skt : g_skt;
	devc->idx = 0;
	devc->max = num;

	if (num <= BASE_SPILL_DEPTH) {
		devc->attr = devc->mark;
		devc->addr = devc->slot;
	} else {
		devc->attr = calloc(sizeof(bool), num);
		devc->addr = calloc(sizeof(struct iovec), num);
	}

	assert(devc->attr);
	assert(devc->addr);
}

void zmqmsg_spill(struct skt_device *devc, char *data, size_t size, bool copy)
{
	assert(devc);
	assert(devc->idx < devc->max);

	bool            *p_attr = &devc->attr[devc->idx];
	struct iovec    *p_addr = &devc->addr[devc->idx];

	*p_attr = copy;
	p_addr->iov_len = size;

	if (copy == true) {
		char *temp = calloc(sizeof(char), size);
		assert(temp);
		memcpy(temp, data, size);
		p_addr->iov_base = temp;
	} else {
		p_addr->iov_base = data;
	}

	devc->idx++;
}

void zmqmsg_flush(struct skt_device *devc)
{
	assert(devc);
	assert(devc->skt);

	int ok = zmq_sendiov(devc->skt, devc->addr, devc->idx, ZMQ_SNDMORE);

	if (ok == -1) {
		printf("%s\n", zmq_strerror(zmq_errno()));
	}

	int i = 0;

	for (i = 0; i < devc->idx; i++) {
		if ((devc->attr[i] == true)
			&& (devc->addr[i].iov_base != NULL)) {
			free(devc->addr[i].iov_base);
		}
	}

	memset(devc->attr, false, sizeof(bool) * devc->idx);
	memset(devc->addr, 0, sizeof(struct iovec) * devc->idx);
	devc->idx = 0;
}

void zmqmsg_close(struct skt_device *devc)
{
	assert(devc);

	int i = 0;

	for (i = 0; i < devc->idx; i++) {
		if ((devc->attr[i] == true)
			&& (devc->addr[i].iov_base != NULL)) {
			free(devc->addr[i].iov_base);
		}
	}

	if (devc->max > BASE_SPILL_DEPTH) {
		free(devc->attr);
		free(devc->addr);
	}
}

/*==============================end zmq=====================================*/

