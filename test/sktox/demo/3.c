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

int main(int argc, char *argv[])
{
	void *context = zmq_ctx_new();

	void    *subscriber = zmq_socket(context, ZMQ_SUB);
	int     rcvhwm = 0;
	int     ok = zmq_setsockopt(subscriber, ZMQ_RCVHWM, &rcvhwm, sizeof(int));

	assert(ok == 0);
	int rc = zmq_connect(subscriber, "tcp://127.0.0.1:5556");
	assert(rc == 0);

	// char *filter = (argc > 1)? argv [1]: "a";
	char *filter = "";
	rc = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, filter, strlen(filter));
	assert(rc == 0);

	int idx = 0;
#ifdef SINGLE_METHOD
	char *buffer = calloc(MAX_MSG_SIZE, 1);

	while (1) {
		int size = zmq_recv(subscriber, buffer, MAX_MSG_SIZE, 0);
		assert(size != -1);
		char *data = strdup(buffer);
		assert(data != NULL);
		free(data);
		idx++;

		if ((idx % 1000000) == 0) {
			printf("100万\n");
		}
	}

	free(buffer);
#else
	while (1) {
		struct iovec ibuffer[32];
		memset(&ibuffer[0], 0, sizeof(ibuffer));
		size_t count = 32;
		rc = zmq_recviov(subscriber, &ibuffer[0], &count, 0);
		count++;
		// printf("======\n");
		int i = 0;

		for (i = 0; i < count; i++) {
			free(ibuffer[i].iov_base);
		}

		idx++;

		if ((idx % 1000000) == 0) {
			printf("100万\n");
		}
	}
#endif	/* ifdef SINGLE_METHOD */

	zmq_close(subscriber);
	zmq_ctx_destroy(context);
	return 0;
}

