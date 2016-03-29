#include "zmq.h"
#include "libmini.h"
#include "send_data.h"

#include <unistd.h>
#include <stdlib.h>

typedef struct send_ctx_t
{
	void    *zmq_ctx;
	void    *zmq_socket;
	MCQueue *qsend;
} send_ctx_t;

#define dtpkt_all_size(size) (sizeof(dtpkt_t) + size)

static send_ctx_t       g_send_ctx;
static void             *thrdSendData(void *args);

int startSendThread(const char *server, int port)
{
	void *ctx = zmq_ctx_new();

	assert(ctx != NULL);

	void *skt = zmq_socket(ctx, ZMQ_REQ);
	assert(skt != NULL);

	char connstr[64];
	sprintf(connstr, "tcp://%s:%d", server, port);

	if (zmq_connect(skt, connstr) != 0) {
		x_printf(E, "zmq_connect Execute fail. Error-%s.\n", zmq_strerror(errno));
		zmq_close(skt);
		zmq_ctx_destroy(ctx);
		return -1;
	}

	int     error;
	MCQueue *qsend = initMCQueue(&error);
	assert(qsend != NULL);

	g_send_ctx.qsend = qsend;
	g_send_ctx.zmq_ctx = ctx;
	g_send_ctx.zmq_socket = skt;

	pthread_t       thrd;
	int             res = pthread_create(&thrd, NULL, thrdSendData, NULL);
	usleep(1000);

	return res;
}

int destroySendThread()
{
	zmq_close(g_send_ctx.zmq_socket);
	zmq_ctx_destroy(g_send_ctx.zmq_ctx);
	destroyMCQueue(g_send_ctx.qsend);

	return 0;
}

void *thrdSendData(void *args)
{
	int     res, errno;
	char    mksure[4];
	dtpkt_t *dtpkt = NULL;

	while (1) {
		dtpkt = popData();

		if (!dtpkt) {
			usleep(8000); continue;
		}

SEND_AGAIN:
		res = zmq_send(g_send_ctx.zmq_socket, dtpkt->dt_data, dtpkt->dt_size, 0);

		if (res == -1) {
			if (errno == EINTR) {
				goto SEND_AGAIN;
			}

			x_printf(E, "SEND:<%s:%d bytes> fail. Error-%s.\n", dtpkt->dt_data, dtpkt->dt_size, zmq_strerror(errno));
			continue;
		}

RECV_AGAIN:
		res = zmq_recv(g_send_ctx.zmq_socket, mksure, sizeof(mksure), 0);

		if (res == -1) {
			if (errno == EINTR) {
				goto RECV_AGAIN;
			}

			x_printf(E, "RECV:<'OK'> fail. Error-%s.\n", zmq_strerror(errno));
			continue;
		}

		free(dtpkt);

		if (strcmp(mksure, "OK") != 0) {
			x_printf(E, "RECV:<RESPONSE> fail. Error-%s.\n", zmq_strerror(errno));
			continue;
		}
	}	/* End while. */
}

int pushData(const void *data, size_t size)
{
	assert(data != NULL);
	assert(size > 0);

	dtpkt_t *dtpkt = (dtpkt_t *)malloc(dtpkt_all_size(size));

	if (!dtpkt) {
		x_printf(E, "pushData: malloc(%s:%lu) bytes fail. Error-%s.\n", (char *)data, (size_t)dtpkt_all_size(dtpkt), strerror(errno));
		return -1;
	}

	memcpy(dtpkt->dt_data, data, size);
	dtpkt->dt_size = size;

	MCQElement *ele = (MCQElement *)malloc(sizeof(MCQElement));

	if (!ele) {
		x_printf(E, "pushData: malloc(MCQElement:%lu) bytes fail. Error-%s.\n", sizeof(MCQElement), strerror(errno));
		free(dtpkt);
		return -1;
	}

	ele->pValue = (void *)dtpkt;
	ele->prev = NULL;
	ele->next = NULL;
	ele->iQEleType = 0;

	enMCQueue(g_send_ctx.qsend, ele);
	return 0;
}

dtpkt_t *popData()
{
	if (mcQueueLength(g_send_ctx.qsend) <= 0) {
		return NULL;
	}

	dtpkt_t         *dtpkt = NULL;
	MCQElement      *ele = NULL;

	deMCQueue(g_send_ctx.qsend, &ele);

	if (!ele) {
		x_printf(W, "popData: deMCQueue(,&ele) fail. ele is NULL.\n");
		return NULL;
	}

	dtpkt = ele->pValue;
	free(ele);

	return dtpkt;
}

