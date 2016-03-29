#include <string.h>
#include <stdio.h>
#include <zmq.h>
int main(int argc, char *argv[])
{
	void    *context = zmq_init(10);// there are 10 io_threads,and prepare for 0MQ context.
	void    *requester = zmq_socket(context, ZMQ_REQ);
	int     rc = zmq_connect(requester, "ipc:///home/baoxue/work/git/cpy/sktox/aa");

	zmq_send(requester, "hello", 5, 0);
	char xx[42] = {};
	zmq_recv(requester, xx, 42, 0);
	printf("Received %s\n", xx);
	return 0;
}

