#pragma once
#include <zmq.h>
#include <sys/uio.h>

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define BASE_SPILL_DEPTH 32

struct skt_device
{
	void            *skt;

	size_t          idx;
	size_t          max;
	bool            *attr;
	struct iovec    *addr;

	bool            mark               [BASE_SPILL_DEPTH];	// base attr
	struct iovec    slot       [BASE_SPILL_DEPTH];		// base addr
};

void zmqskt_init(char *host, int port);

void zmqskt_exit(void);

void zmqmsg_start(struct skt_device *devc, void *skt, size_t num);

void zmqmsg_spill(struct skt_device *devc, char *data, size_t size, bool copy);

void zmqmsg_flush(struct skt_device *devc);

void zmqmsg_close(struct skt_device *devc);

