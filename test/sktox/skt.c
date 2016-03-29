#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <assert.h>

#include "skt.h"

/*
 * 1.直接调用的资源必须在同一个进程内使用.
 * 2.跨进程通信时，上下文必须重新创建.
 */

static char     *g_own = "my";
static void     *g_ctx = NULL;

void skt_register(char *name)
{
	if (name) {
		g_own = name;
	}

	assert(!g_ctx);
	g_ctx = zmq_ctx_new();
}

void *zmq_process_start(void *(*fcb)(void *args), void *args)
{
	pid_t pid = 0;

	if ((pid = fork()) < 0) {
		perror("fork");
		exit(-1);
	} else if (pid == 0) {
		g_ctx = zmq_ctx_new();	/*must*/
		fcb(args);
		exit(0);
	} else {
		printf("FORK ONE PROCESS -->PID :%d\n", pid);
	}

	return NULL;
}

/*===============================================================================*/
static void     *g_sender = NULL;
static void     *g_recver = NULL;

static void *_cli_proxy_task(void *args)
{
	zmq_proxy(g_recver, g_sender, NULL);
}

void zmq_cli_init(char *host, int port)
{	// FIXME:safe and once
	g_sender = zmq_socket(g_ctx, ZMQ_PUSH);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	int rc = zmq_connect(g_sender, addr);
	assert(rc == 0);

	g_recver = zmq_socket(g_ctx, ZMQ_PULL);
#ifdef SELECT_MULTITHREAD
	rc = zmq_bind(g_recver, "inproc://backend");
#else
	memset(addr, 0, 64);
	sprintf(addr, "ipc://%s-cloud.ipc", g_own);
	rc = zmq_bind(g_recver, addr);
#endif
	assert(rc == 0);

	zmq_threadstart(_cli_proxy_task, NULL);
}

void zmq_cli_spill(struct skt_device *devc, char *data, size_t size)
{
	assert(devc);
	assert(devc->idx < MAX_SPILL_DEPTH);

	if (!devc->skt) {
		devc->skt = zmq_socket(g_ctx, ZMQ_PUSH);
#ifdef SELECT_MULTITHREAD
		int ok = zmq_connect(devc->skt, "inproc://backend");
#else
		char addr[64] = {};
		sprintf(addr, "ipc://%s-cloud.ipc", g_own);
		int ok = zmq_connect(devc->skt, addr);
#endif

		if (ok == -1) {
			printf("%s\n", zmq_strerror(zmq_errno()));
		}
	}

	devc->ibuffer[devc->idx].iov_base = data;
	devc->ibuffer[devc->idx].iov_len = size;
	devc->idx++;
}

void zmq_cli_flush(struct skt_device *devc)
{
	int ok = zmq_sendiov(devc->skt, &devc->ibuffer[0], devc->idx, ZMQ_SNDMORE);

	if (ok == -1) {
		printf("%s\n", zmq_strerror(zmq_errno()));
	}

	devc->idx = 0;
	memset(devc->ibuffer, 0, sizeof(devc->ibuffer));
}

void zmq_cli_exit(void)
{	// FIXME:safe and once
	zmq_close(g_sender);
	zmq_close(g_recver);
	zmq_ctx_destroy(g_ctx);
}

/*===============================================================================*/
struct skt_kernel       *g_skt_recver = NULL;
struct skt_kernel       *g_skt_router = NULL;

/*
 *   void skt_proxy (void *recver, void *sender)
 *   {
 *        struct iovec ibuffer[32] ;
 *        while (1) {
 *                memset(&ibuffer[0], 0, sizeof(ibuffer));
 *                size_t count = 32;
 *                int rc = zmq_recviov (recver, &ibuffer[0], &count, 0);
 *                if (rc == -1){
 *                        printf("%s\n", zmq_strerror(zmq_errno()));
 *                }else{
 *                        count ++;
 *                        rc = zmq_sendiov (sender, &ibuffer[0], count, ZMQ_SNDMORE);
 *                        int i = 0;
 *                        for(i=0; i<count; i++){
 *                                free (ibuffer[i].iov_base);
 *                        }
 *                }
 *        }
 *
 *   }
 */
void zmq_trs_init(void)
{	// FIXME:safe and once
}

void zmq_trs_add_inport(int port)
{
	void                    *receiver = zmq_socket(g_ctx, ZMQ_PULL);
	struct skt_kernel       *recver = calloc(sizeof(struct skt_kernel), 1);
	struct skt_kernel       *history = g_skt_recver;

	recver->skt = receiver;
	recver->next = history;
	g_skt_recver = recver;

	char addr[64] = {};
	sprintf(addr, "tcp://*:%d", port);
	int rc = zmq_bind(receiver, addr);
	assert(rc == 0);
}

void zmq_trs_add_export(int port)
{
	void                    *publisher = zmq_socket(g_ctx, ZMQ_PUB);
	struct skt_kernel       *router = calloc(sizeof(struct skt_kernel), 1);

	if (g_skt_router == NULL) {
		router->skt = publisher;
		router->next = router;
		g_skt_router = router;
	} else {
		struct skt_kernel *history = g_skt_router->next;
		router->skt = publisher;
		router->next = history;
		g_skt_router->next = router;
	}

	int     sndhwm = 0;
	int     ok = zmq_setsockopt(publisher, ZMQ_SNDHWM, &sndhwm, sizeof(int));
	assert(ok == 0);
	// uint64_t swap = 25000000;
	// zmq_setsockopt (publisher, ZMQ_SWAP, &swap, sizeof (swap));
	char addr[64] = {};
	sprintf(addr, "tcp://*:%d", port);
	int rc = zmq_bind(publisher, addr);
	assert(rc == 0);
}

void zmq_trs_start(void)
{
	int                     idx = 0;
	int                     max = 0;
	struct skt_kernel       *temp = g_skt_recver;

	while (temp) {
		max++;
		temp = temp->next;
	}

	zmq_pollitem_t items [max];

	for (idx = 0, temp = g_skt_recver; idx < max; idx++, temp = temp->next) {
		items[idx].socket = temp->skt;
		items[idx].fd = 0;
		items[idx].events = ZMQ_POLLIN;
		items[idx].revents = 0;
	}

	struct iovec ibuffer[32];

	while (1) {
		zmq_poll(items, max, -1);

		for (idx = 0; idx < max; idx++) {
			if (items [idx].revents & ZMQ_POLLIN) {
				// printf("idx %d\n", idx);
				memset(&ibuffer[0], 0, sizeof(ibuffer));
				size_t  count = 32;
				int     rc = zmq_recviov(items[idx].socket, &ibuffer[0], &count, 0);

				if (rc == -1) {
					printf("%s\n", zmq_strerror(zmq_errno()));
				} else {
					count++;
					rc = zmq_sendiov(g_skt_router->skt, &ibuffer[0], count, ZMQ_SNDMORE);
					int i = 0;

					for (i = 0; i < count; i++) {
						free(ibuffer[i].iov_base);
					}

					g_skt_router = g_skt_router->next;
				}
			}
		}
	}
}

void zmq_trs_exit(void)
{	// FIXME:safe and once
	// zmq_close (g_receiver);
	zmq_ctx_destroy(g_ctx);
}

/*===============================================================================*/
void    *g_subscriber = NULL;
void    *g_forwarder = NULL;

void zmq_srv_init(char *host, int port)
{	// FIXME:safe and once
	g_subscriber = zmq_socket(g_ctx, ZMQ_SUB);
	int     rcvhwm = 0;
	int     ok = zmq_setsockopt(g_subscriber, ZMQ_RCVHWM, &rcvhwm, sizeof(int));
	assert(ok == 0);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	int rc = zmq_connect(g_subscriber, addr);
	assert(rc == 0);
	// char *filter = (argc > 1)? argv [1]: "a";
	char *filter = "";
	rc = zmq_setsockopt(g_subscriber, ZMQ_SUBSCRIBE, filter, strlen(filter));
	assert(rc == 0);

	g_forwarder = zmq_socket(g_ctx, ZMQ_PUSH);
#ifdef SELECT_MULTITHREAD
	rc = zmq_bind(g_forwarder, "inproc://backend");
#else
	memset(addr, 0, 64);
	sprintf(addr, "ipc://%s-sault.ipc", g_own);
	rc = zmq_bind(g_forwarder, addr);
#endif
	assert(rc == 0);
}

void zmq_srv_fetch(struct skt_device *devc)
{
	assert(devc);

	if (!devc->skt) {
		devc->skt = zmq_socket(g_ctx, ZMQ_PULL);
#ifdef SELECT_MULTITHREAD
		int ok = zmq_connect(devc->skt, "inproc://backend");
#else
		char addr[64] = {};
		sprintf(addr, "ipc://%s-sault.ipc", g_own);
		int ok = zmq_connect(devc->skt, addr);
#endif

		if (ok == -1) {
			printf("%s\n", zmq_strerror(zmq_errno()));
		}
	}

	memset(devc->ibuffer, 0, sizeof(devc->ibuffer));
	size_t  count = MAX_SPILL_DEPTH;
	int     rc = zmq_recviov(devc->skt, devc->ibuffer, &count, 0);
	devc->idx = count + 1;
}

void zmq_srv_start(void)
{
	zmq_proxy(g_subscriber, g_forwarder, NULL);
}

void zmq_srv_exit(void)
{	// FIXME:safe and once
	zmq_close(g_subscriber);
	zmq_ctx_destroy(g_ctx);
}

