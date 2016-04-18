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
void    *g_collecter = NULL;
void    *g_forwarder = NULL;

void zmq_srv_init(char *host, int port)
{
	g_collecter = zmq_socket(g_ctx, ZMQ_PULL);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	int rc = zmq_connect(g_collecter, addr);
	assert(rc == 0);
	printf("zmq connect to localhost:6992 success\n");

	g_forwarder = zmq_socket(g_ctx, ZMQ_PUSH);
#ifdef SELECT_MULTITHREAD
	rc = zmq_bind(g_forwarder, "inproc://backend");
#else
	memset(addr, 0, 64);
	sprintf(addr, "ipc://%s-sault.ipc", g_own);
	rc = zmq_bind(g_forwarder, addr);
#endif
	assert(rc == 0);
	printf("zmq bind my-sault.ipc success\n");
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
	printf("before zmq_recv\n");
	int rc = zmq_recviov(devc->skt, devc->ibuffer, &count, 0);
	//int rc = zmq_recv(devc->skt, devc->ibuffer, count, 0);
	printf("after zmq_recv\n");
	devc->idx = count;
}

void zmq_srv_start(void)
{
	zmq_proxy(g_collecter, g_forwarder, NULL);
}

void zmq_srv_exit(void)
{
	zmq_close(g_collecter);
	zmq_close(g_forwarder);
	zmq_ctx_destroy(g_ctx);
}

