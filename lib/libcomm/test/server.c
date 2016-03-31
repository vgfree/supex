#include "communication.h"

#define		EPOLLSIZE	1024

void finished(struct commctx* commctx, int fd, void* usr)
{
	printf("%d fd have action, data is: %s", fd, (char*)usr);
}

int main(int argc, char* argv[])
{
	int i = 0;
	int retval = -1;
	char *data[5] = {"usr data one", "usr data two", "usr data three", "usr data four", "usr data five"};

	if( unlikely(argc < 3) ){
		printf("usage: %s <ipaddr> <port>", argv[0]);
		return -1;
	}

	struct comm *commctx = NULL;
	commctx = comm_ctx_create(EPOLLSIZE);
	if( unlikely(!commctx) ){
		return retval;
	}

	struct cbinfo  finishedcb = {};
	finishedcb.callback = finished;
	finishedcb.usr = data[0];

	retval = comm_socket(commctx, argv[1], argv[2], finishedcb, COMM_BIND);
	if(retval == -1){
		return retval;
	}

	while( 1 ){
		char buff[128] = "nothing is improtant\r\m\t";
		retval = comm_recv(commctx, fd, buff, strlen(buff));
		if( unlikely(retval < 0) ){
			return retval;
		}
		retval = comm_send(commctx, fd, data[i], strlen(data[i]));
		if( unlikely(retval < 0) )
		{
			return retval;
		}
		i = (i+1)%5;
	}
}
