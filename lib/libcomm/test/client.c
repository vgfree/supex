#include "../communication.h"

#define		EPOLLSIZE	1024

void close_fun()
{
	printf("client here is close_fun()\n");
}

void write_fun()
{
	printf("client here is write_fun()\n");
}

void read_fun()
{
	printf("client here is read_fun()\n");
}

void accept_fun()
{
	printf("client here is accept_fun()\n");
}

void timeout_fun()
{
	printf("client here is timeout_fun()\n");
}

/* @status: FD_CLOSE, FD_READ, FD_WRITE, FD_ACCEPT */
void event_fun(struct comm_context* commctx, struct portinfo *portinfo, void* usr)
{
	switch (portinfo->stat)
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
		case FD_INIT:
			accept_fun(); /* 当fd的类型为COMM_ACCEPT时才是accept事件 */
		default:
			timeout_fun();
			break;
	}
	log("client %d fd have action\n", portinfo->fd);
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
		char buff[128] = "nothing is important\r\t";
		char content[1024] = {};

		struct comm_message sendmsg = {fd, -1, -1, strlen(buff), buff};
		
		retval = comm_send(commctx, &sendmsg, false, -1);
		if( unlikely(retval < 0) ) {
			log("clinet comm_send failed\n");
			continue ;
		}
		log("clinet comm_send successed, message:%s\n", sendmsg.content);
		struct comm_message recvmsg = {};
		recvmsg.content = content;
		while (1) {
			retval = comm_recv(commctx, &recvmsg, false, -1);
			if( unlikely(retval < 0) ){
				sleep(1);
				log("client recv_data failed\n");
			} else {
				log("client recv_data successed, message:%s\n", recvmsg.content);
				break ;
			}
		}
	}
	comm_ctx_destroy(commctx);
}
