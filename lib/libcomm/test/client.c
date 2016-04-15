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
	int i = 0;
	int retval = -1;
	int fd = -1;
	char *data[5] = {"usr data one", "usr data two", "usr data three", "usr data four", "usr data five"};

	if( unlikely(argc < 3) ){
		printf("usage: %s <ipaddr> <port>", argv[0]);
		return -1;
	}

	struct comm_context *commctx = NULL;
	commctx = comm_ctx_create(EPOLLSIZE);
	if( unlikely(!commctx) ){
		log("client comm_ctx_create failed\n");
		return retval;
	}
	log("client comm_ctx_create successed\n");

	struct cbinfo  finishedcb = {};
	finishedcb.callback = event_fun;
	finishedcb.usr = data[0];

	fd = comm_socket(commctx, argv[1], argv[2], &finishedcb, COMM_CONNECT);
	if(fd == -1){
		log("client comm_socket failed\n");
		return retval;
	}
	log("client comm_socket successed\n");

	while( 1 ){
		char buff[128] = "nothing is improtant\r\t";
		char content[1024] = {};

		struct comm_message sendmsg = {fd, -1, -1, strlen(buff), buff};
		sleep(3);
		
		retval = comm_send(commctx, &sendmsg);
		if( unlikely(retval < 0) ) {
			log("clinet comm_send failed\n");
			return retval;
		}
		log("clinet comm_send successed, message:%s\n", sendmsg.content);
		struct comm_message recvmsg = {};
		recvmsg.content = content;
		retval = comm_recv(commctx, &recvmsg);
		if( unlikely(retval < 0) ){
			log("client recv_data failed\n");
			return retval;
		}
		log("client recv_data successed, message:%s\n", recvmsg.content);
		i = (i+1)%5;
	}
}
