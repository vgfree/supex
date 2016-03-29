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

int main(void)
{
	zctx_t *ctx = zctx_new();

	void *publisher = zsocket_new(ctx, ZMQ_PUB);

	zsocket_set_sndhwm(publisher, 0);
	// zsocket_set_swap (publisher, 25000000);
	int rc = zsocket_bind(publisher, "tcp://*:5556");
	assert(rc == 5556);

	void *receiver = zsocket_new(ctx, ZMQ_PULL);
	rc = zsocket_bind(receiver, "tcp://*:5558");
	assert(rc == 5558);

#if 0
	zproxy_t *proxy = zproxy_new(ctx, receiver, publisher);
	assert(proxy);
  #if 0
	void *capture = zsocket_new(ctx, ZMQ_PULL);
	assert(capture);
	rc = zsocket_bind(capture, "inproc://capture");
	assert(rc == 0);

	zproxy_capture(proxy, "inproc://capture");
  #endif

	while (1) {
		sleep(1);
	}

	zproxy_destroy(&proxy);
#else
	int idx = 0;

	while (1) {
		zframe_t *frame = zframe_recv(receiver);
		assert(frame);
		int     more = zframe_more(frame);
		int     ok = zframe_send(&frame, publisher, more);
		assert(ok == 0);

		if (!more) {
			idx++;

			if ((idx % 1000000) == 0) {
				printf("100万\n");
			}
		}
	}
#endif	/* if 0 */
	zsocket_destroy(ctx, receiver);
	zsocket_destroy(ctx, publisher);
	zctx_destroy(&ctx);
	return 0;
}

// zmq_device (ZMQ_QUEUE, frontend, backend);
// vi mspoller.c
// mtserver: Multithreaded service in C**

