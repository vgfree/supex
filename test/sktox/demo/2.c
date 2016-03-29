#include <zmq.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <assert.h>

// XSI vector I/O
#if defined ZMQ_HAVE_UIO
  #include <sys/uio.h>
#else
struct iovec
{
	void    *iov_base;
	size_t  iov_len;
};
#endif

#define MAX_MSG_SIZE 1024 * 8

int main(void)
{
	void *context = zmq_ctx_new();

	void    *publisher = zmq_socket(context, ZMQ_PUB);
	int     sndhwm = 0;
	int     ok = zmq_setsockopt(publisher, ZMQ_SNDHWM, &sndhwm, sizeof(int));

	assert(ok == 0);
	// uint64_t swap = 25000000;
	// zmq_setsockopt (publisher, ZMQ_SWAP, &swap, sizeof (swap));
	int rc = zmq_bind(publisher, "tcp://*:5556");
	assert(rc == 0);

	void *receiver = zmq_socket(context, ZMQ_PULL);
	rc = zmq_bind(receiver, "tcp://*:5558");
	assert(rc == 0);

#if 1
	zmq_proxy(receiver, publisher, NULL);
#else
	int     idx = 0;
	char    *buffer = calloc(MAX_MSG_SIZE, 1);

	while (1) {
  #ifdef SINGLE_METHOD
		int     rsize = zmq_recv(receiver, buffer, MAX_MSG_SIZE, 0);
		int     ssize = zmq_send(publisher, buffer, rsize, 0);
		assert(rsize == ssize);
  #else
		struct iovec ibuffer[32];
		memset(&ibuffer[0], 0, sizeof(ibuffer));
		size_t count = 32;
		rc = zmq_recviov(receiver, &ibuffer[0], &count, 0);
		assert(rc != -1);
		// printf("======\n");
		count++;
		rc = zmq_sendiov(publisher, &ibuffer[0], count, ZMQ_SNDMORE);
		int i = 0;

		for (i = 0; i < count; i++) {
			free(ibuffer[i].iov_base);
		}
  #endif

		idx++;

		if ((idx % 1000000) == 0) {
			printf("100万\n");
		}
	}
#endif	/* if 1 */
	zmq_close(receiver);
	zmq_close(publisher);
	zmq_ctx_destroy(context);
	return 0;
}

// zmq_device (ZMQ_QUEUE, frontend, backend);
// vi mspoller.c
// mtserver: Multithreaded service in C**

