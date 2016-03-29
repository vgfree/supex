#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
int main(void)
{
	void *context = zmq_init(1);

	//  Socket to talk to clients
	void *responder = zmq_socket(context, ZMQ_REP);

	//  zmq_bind (responder, "tcp://*:5555");
	zmq_bind(responder, "ipc:///home/baoxue/work/git/cpy/sktox/aa");

	while (1) {
		//  Wait for next request from client
		char xx[42] = {};
		zmq_recv(responder, xx, 42, 0);
		printf("Received Hello\n");

		//  Do some 'work'
		sleep(1);

		//  Send reply back to client
		zmq_send(responder, xx, strlen(xx), 0);
	}

	//  We never get here but if we did, this would be how we end
	zmq_close(responder);
	zmq_term(context);
	return 0;
}

