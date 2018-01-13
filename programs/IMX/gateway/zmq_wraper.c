#include <assert.h>
#include "zmq.h"

#include "zmq_wraper.h"


void *zmq_push_init(void *ctx, char *host, int port)
{
	assert(ctx);
	void *sock = zmq_socket(ctx, ZMQ_PUSH);

	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	int rc = zmq_bind(sock, addr);
	assert(rc == 0);
	return sock;
}

void *zmq_pull_init(void *ctx, char *host, int port)
{
	assert(ctx);
	void *sock = zmq_socket(ctx, ZMQ_PULL);

	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	x_printf(D, "addr:%s.", addr);
	int rc = zmq_bind(sock, addr);
	assert(rc == 0);
	return sock;
}
