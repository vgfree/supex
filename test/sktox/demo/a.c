#include <czmq.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <assert.h>

char    data1[] = "a1234567890";
char    data2[] = "a1234567890sadasasld;a;dsa;f;sa;sa;sa,dsa.d,sa/d,sa/d,sa/d,saf,sa;fd,sa;d,sa;d,sa/,ds;dsa;d,sad,sa;d,as,da;d,as;,dsa,dsaddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd";

int main(void)
{
	//  Socket to receive messages on
	zctx_t *ctx = zctx_new();

	//  Socket to send messages to
	void *sender = zsocket_new(ctx, ZMQ_PUSH);

	zsocket_connect(sender, "tcp://127.0.0.1:5558");

	//  Process tasks forever
	int xx = 0;
	printf("size %d\n", sizeof(data1) + sizeof(data2));

	while (xx <= 2000000) {
		int rc = zsocket_sendmem(sender, data1, strlen(data1), ZFRAME_MORE);
		assert(rc == 0);
		rc = zsocket_sendmem(sender, data2, strlen(data2), 0);
		assert(rc == 0);
		// sleep(1);
		xx++;
	}

	// sleep(20);
	sleep(10);
	zsocket_destroy(ctx, sender);
	zctx_destroy(&ctx);
	return 0;
}

