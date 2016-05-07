#include "../communication.h"

#define		EPOLLSIZE	1024

void close_fun()
{
	printf("server here is close_fun()\n");
}

void write_fun()
{
	printf("server here is write_fun()\n");
}

void read_fun()
{
	printf("server here is read_fun()\n");
}

void accept_fun()
{
	printf("server here is accept_fun()\n");
}

void timeout_fun()
{
	printf("server here is timeout_fun()\n");
}

/* @status: FD_CLOSE, FD_READ, FD_WRITE, FD_ACCEPT */
void event_fun(struct comm_context* commctx, struct portinfo *portinfo, void* usr)
{
	switch (portinfo->stat)
	{
		case FD_CLOSE:
			close_fun();
			break ;
		case FD_WRITE:
			write_fun();
			break ;
		case FD_READ:
			read_fun();
			break ;
		case FD_INIT:
			if (portinfo->type == COMM_ACCEPT) {
				accept_fun(); /* 当fd的类型为COMM_ACCEPT时才是accept事件 */
			}
			break ;
		default:
			timeout_fun();
			break ;
	}
	log("server %d fd have action\n",portinfo->fd);
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

	while( 1 ) {
		char buff[128] = "everything is improtant\r\t";
		char content[1024] = {};
		struct comm_message recvmsg = {};
		recvmsg.content = content;
		while (1) {
			retval = comm_recv(commctx, &recvmsg, false, -1);
			if( unlikely(retval < 0) ){
				sleep(1);
				log("comm_recv failed\n");
			} else {
				log("comm_recv success, size: %d message:%s\n", recvmsg.package.dsize, recvmsg.content);
				break ;
			}
		}
		struct comm_message sendmsg = {0};
		sendmsg.fd = recvmsg.fd;
		/* 高四位是压缩设置 低四位是加密设置 */
		sendmsg.config = NO_COMPRESSION | NO_ENCRYPTION;
		sendmsg.socket_type = REP_METHOD;
		sendmsg.content = buff;
		sendmsg.package.dsize = strlen(buff);
		sendmsg.package.frames = 1;
		sendmsg.package.packages = 1;
		sendmsg.package.frame_size[0] = strlen(buff);
		sendmsg.package.frame_offset[0] = 0;
		sendmsg.package.frames_of_package[0] = 1;
		
		retval = comm_send(commctx, &sendmsg, false, -1);
		if( unlikely(retval < 0) )
		{
			log("comm_send failed\n");
		} else {
			log("comm_send data successed, message:%s\n", sendmsg.content);
		}
	}
	comm_ctx_destroy(commctx);
}
