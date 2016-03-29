#include <czmq.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <assert.h>

#define MAX_MSG_SIZE 1024 * 8

int main(int argc, char *argv[])
{
	zctx_t *ctx = zctx_new();

	void *subscriber = zsocket_new(ctx, ZMQ_SUB);

	zsocket_set_rcvhwm(subscriber, 0);
	int rc = zsocket_connect(subscriber, "tcp://127.0.0.1:5556");
	assert(rc == 0);

	// char *filter = (argc > 1)? argv [1]: "a";
	char *filter = "";
	zsock_set_subscribe(subscriber, filter);

	int     idx = 0;
	char    *buffer = calloc(MAX_MSG_SIZE, 1);

	while (1) {
		size_t  size = 0;
		int     more = 0;
		do {
			zframe_t *frame = zframe_recv(subscriber);
			assert(frame);
			assert(zframe_is(frame));
			int rsize = zframe_size(frame);
			assert(rsize != -1);
			assert((size + rsize) <= MAX_MSG_SIZE);
			byte *data = zframe_data(frame);
			memcpy(buffer + size, data, rsize);
			size += rsize;

			more = zframe_more(frame);
			zframe_destroy(&frame);
		} while (more);
		char *data = strdup(buffer);
		assert(data);
		free(data);
		idx++;

		if ((idx % 1000000) == 0) {
			printf("100万\n");
		}
	}

	zsocket_destroy(ctx, subscriber);
	zctx_destroy(&ctx);
	return 0;
}

