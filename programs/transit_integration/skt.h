#pragma once

#include <zmq.h>

#include <sys/uio.h>

#define MAX_SPILL_DEPTH 32

struct skt_device
{
	int             idx;
	void            *skt;
	struct iovec    ibuffer[MAX_SPILL_DEPTH];
};
struct skt_kernel
{
	void                    *skt;
	struct skt_kernel       *next;
};

void skt_register(char *name);

void *zmq_process_start(void *(*fcb)(void *args), void *args);

void zmq_srv_init(char *host, int port);

void zmq_Javasrv_init(void **);

void zmq_srv_fetch(struct skt_device *devc);

void zmq_srv_start(void);

void zmq_srv_exit(void);

