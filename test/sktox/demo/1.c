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

char    data1[] = "a1234567890";
char    data2[] = "a1234567890sadasasld;a;dsa;f;sa;sa;sa,dsa.d,sa/d,sa/d,sa/d,saf,sa;fd,sa;d,sa;d,sa/,ds;dsa;d,sad,sa;d,as,da;d,as;,dsa,dsaddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd";

int main(void)
{
	//  Socket to receive messages on
	void *context = zmq_ctx_new();

	//  Socket to send messages to
	void *sender = zmq_socket(context, ZMQ_PUSH);

	zmq_connect(sender, "tcp://127.0.0.1:5558");

	//  Process tasks forever
	int xx = 0;
	printf("size %d\n", sizeof(data1) + sizeof(data2));
#ifdef SINGLE_METHOD
	while (xx <= 2000000) {
		int size = zmq_send(sender, data1, strlen(data1), 0);
#else
	struct iovec ibuffer[32];

	while (xx <= 2000000) {
		// while ( xx <= 1 ) {
		memset(&ibuffer[0], 0, sizeof(ibuffer));
		ibuffer[0].iov_base = data1;
		ibuffer[0].iov_len = strlen(data1);
		ibuffer[1].iov_base = data2;
		ibuffer[1].iov_len = strlen(data2);
		int rc = zmq_sendiov(sender, &ibuffer[0], 2, ZMQ_SNDMORE);
#endif
		// sleep(1);
		xx++;
	}

	// sleep(20);
	sleep(10);
	zmq_close(sender);
	zmq_ctx_destroy(context);
	return 0;
}

