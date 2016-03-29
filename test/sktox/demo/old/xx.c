#include <zmq.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <assert.h>

#include "x.h"

static void *g_ctx = NULL;
/*===============================================================================*/
static void     *g_sender = NULL;
static void     *g_recver = NULL;

static void *_cli_proxy_task(void *args)
{
	g_recver = zmq_socket(g_ctx, ZMQ_PULL);
	int rc = zmq_bind(g_recver, "inproc://backend");
	assert(rc == 0);

	zmq_proxy(g_recver, g_sender, NULL);
}

void zmq_cli_pthread_init(char *host, int port)
{	// FIXME:safe and once
	assert(!g_ctx);
	g_ctx = zmq_ctx_new();
	g_sender = zmq_socket(g_ctx, ZMQ_PUSH);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	int rc = zmq_connect(g_sender, addr);
	assert(rc == 0);

	zmq_threadstart(_cli_proxy_task, NULL);
}

void zmq_cli_pthread_spill(struct skt_device *devc, char *data, size_t size)
{
	assert(devc);
	assert(devc->idx < MAX_SPILL_DEPTH);

	if (!devc->skt) {
		devc->skt = zmq_socket(g_ctx, ZMQ_PUSH);
		zmq_connect(devc->skt, "inproc://backend");
	}

	devc->ibuffer[devc->idx].iov_base = data;
	devc->ibuffer[devc->idx].iov_len = size;
	devc->idx++;
}

void zmq_cli_pthread_flush(struct skt_device *devc)
{
	zmq_sendiov(devc->skt, &devc->ibuffer[0], devc->idx, ZMQ_SNDMORE);

	devc->idx = 0;
	memset(devc->ibuffer, 0, sizeof(devc->ibuffer));
}

void zmq_cli_pthread_exit(void)
{	// FIXME:safe and once
	zmq_close(g_sender);
	zmq_close(g_recver);
	zmq_ctx_destroy(g_ctx);
}

/*===============================================================================*/
void    *g_receiver = NULL;
void    *g_router = NULL;
void    *g_controller = NULL;

static void *_trs_supply_task(void *args)
{
	void    *publisher = zmq_socket(g_ctx, ZMQ_PUB);
	int     sndhwm = 0;
	int     ok = zmq_setsockopt(publisher, ZMQ_SNDHWM, &sndhwm, sizeof(int));

	assert(ok == 0);
	// uint64_t swap = 25000000;
	// zmq_setsockopt (publisher, ZMQ_SWAP, &swap, sizeof (swap));
	char addr[64] = {};
	sprintf(addr, "tcp://*:%d", args);
	int rc = zmq_bind(publisher, addr);
	assert(rc == 0);

	// zmq_proxy (g_router, publisher, NULL);//FIXME
	struct iovec ibuffer[32];

	while (1) {
		memset(&ibuffer[0], 0, sizeof(ibuffer));
		size_t count = 32;
		rc = zmq_recviov(g_router, &ibuffer[0], &count, 0);
		assert(rc != -1);
		printf("======\n");
		count++;
		rc = zmq_sendiov(publisher, &ibuffer[0], count, ZMQ_SNDMORE);
		int i = 0;

		for (i = 0; i < count; i++) {
			free(ibuffer[i].iov_base);
		}
	}
}

static void *_trs_occupy_task(void *args)
{
	void    *publisher = zmq_socket(g_ctx, ZMQ_PUB);
	int     sndhwm = 0;
	int     ok = zmq_setsockopt(publisher, ZMQ_SNDHWM, &sndhwm, sizeof(int));

	assert(ok == 0);
	// uint64_t swap = 25000000;
	// zmq_setsockopt (publisher, ZMQ_SWAP, &swap, sizeof (swap));
	char addr[64] = {};
	sprintf(addr, "tcp://*:%d", args);
	int rc = zmq_bind(publisher, addr);
	assert(rc == 0);

	void *controller = zmq_socket(g_ctx, ZMQ_SUB);
	assert(controller);
	assert(zmq_connect(controller, "inproc://control") == 0);
	assert(zmq_setsockopt(controller, ZMQ_SUBSCRIBE, "", 0) == 0);

	// zmq_proxy_steerable (g_receiver, publisher, NULL, controller);
	zmq_pollitem_t  items[] = {
		{ g_receiver, 0, ZMQ_POLLIN, 0 },
		{ controller, 0, ZMQ_POLLIN, 0 }
	};
	struct iovec    ibuffer[32];

	while (1) {
		zmq_poll(items, 2, -1);

		if (items [0].revents & ZMQ_POLLIN) {
			memset(&ibuffer[0], 0, sizeof(ibuffer));
			size_t count = 32;
			rc = zmq_recviov(g_receiver, &ibuffer[0], &count, 0);
			assert(rc != -1);
			// printf("======\n");
			count++;
			rc = zmq_sendiov(publisher, &ibuffer[0], count, ZMQ_SNDMORE);
			int i = 0;

			for (i = 0; i < count; i++) {
				free(ibuffer[i].iov_base);
			}
		}

		if (items [1].revents & ZMQ_POLLIN) {
			break;
		}
	}

	printf("=====\n");
	zmq_close(publisher);
	sleep(1);
	_trs_supply_task(args);
}

static void *_trs_proxy_task(void *args)
{
	g_router = zmq_socket(g_ctx, ZMQ_ROUTER);
	int rc = zmq_bind(g_router, "inproc://backend");
	assert(rc == 0);

	zmq_proxy(g_receiver, g_router, NULL);
}

void zmq_trs_pthread_set_inport(int port)
{	// FIXME:safe and once
	assert(!g_ctx);
	g_ctx = zmq_ctx_new();

	g_receiver = zmq_socket(g_ctx, ZMQ_PULL);
	char addr[64] = {};
	sprintf(addr, "tcp://*:%d", port);
	int rc = zmq_bind(g_receiver, addr);
	assert(rc == 0);
}

void zmq_trs_pthread_add_export(int port)
{
	static int idx = 0;

	switch (idx++)
	{
		case 0:
			zmq_threadstart(_trs_occupy_task, port);
			break;

		case 1:
			sleep(1);
			g_controller = zmq_socket(g_ctx, ZMQ_PUB);
			assert(g_controller);
			assert(zmq_bind(g_controller, "inproc://control") == 0);

			// assert (zmq_send (g_controller, "PAUSE", 5, 0));
			// assert (zmq_send (g_controller, "RESUME", 6, 0));
			assert(zmq_send(g_controller, "TERMINATE", 9, 0));
			zmq_threadstart(_trs_proxy_task, NULL);
			sleep(1);
			zmq_threadstart(_trs_supply_task, port);
			break;

		default:
			zmq_threadstart(_trs_supply_task, port);
			break;
	}
}

void zmq_trs_pthread_exit(void)
{	// FIXME:safe and once
	zmq_close(g_receiver);
	zmq_close(g_controller);
	zmq_ctx_destroy(g_ctx);
}

// zmq_cli_process_init();

