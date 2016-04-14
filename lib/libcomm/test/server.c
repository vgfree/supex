#include "../communication.h"

#define		EPOLLSIZE	1024

void close_fun()
{
	printf("here is close_fun()\n");
}

void write_fun()
{
	printf("here is write_fun()\n");
}

void read_fun()
{
	printf("here is read_fun()\n");
}

void accept_fun()
{
	printf("here is accept_fun()\n");
}

void timeout_fun()
{
	printf("here is timeout_fun()\n");
}

/* @status: FD_CLOSE, FD_READ, FD_WRITE, FD_ACCEPT */
void event_fun(struct comm_context* commctx, int fd, int status, void* usr)
{
	switch (status)
	{
		case FD_CLOSE:
			close_fun();
			break;
		case FD_WRITE:
			write_fun();
			break;
		case FD_READ:
			read_fun();
			break;
		case FD_ACCEPT:
			accept_fun();
			break;
		case FD_TIMEOUT:
			timeout_fun();
			break;
		default:
			break;
	}
	printf("%d fd have action, data is: %s", fd, (char*)usr);
}

int main(int argc, char* argv[])
{
	log("GCC_VERSION:%d\n", GCC_VERSION);
	int i = 0;
	int retval = -1;
	char *data[5] = {"usr data one", "usr data two", "usr data three", "usr data four", "usr data five"};

	if( unlikely(argc < 3) ){
		printf("usage: %s <ipaddr> <port>", argv[0]);
		return -1;
	}

	struct comm_context *commctx = NULL;
	commctx = comm_ctx_create(EPOLLSIZE);
	if( unlikely(!commctx) ){
		log("comm_ctx_create failed\n");
		return retval;
	}

	log("comm_ctx_create successed\n");
	struct cbinfo  finishedcb = {};
	finishedcb.callback = event_fun;
	finishedcb.usr = data[0];

	retval = comm_socket(commctx, argv[1], argv[2], &finishedcb, COMM_BIND);
	if(retval == -1){
		log("comm_socket failed\n");
		return retval;
	}
	log("comm_socket COMM_BIND successed\n");

	while( 1 ){
		char buff[128] = "nothing is improtant\r\t";
		char content[1024] = {};
		struct comm_message recvmsg = {};
		recvmsg.content = content;
		retval = comm_recv(commctx, &recvmsg);
		if( unlikely(retval < 0) ){
			log("comm_recv failed, probably becasue don't have any data\n");
			//return retval;
			while(1) {
			} ;
		}
		struct comm_message sendmsg = {recvmsg.fd, -1, -1, recvmsg.size, buff};
		
		retval = comm_send(commctx, &sendmsg);
		if( unlikely(retval < 0) )
		{
			return retval;
		}
		i = (i+1)%5;
	}
}
