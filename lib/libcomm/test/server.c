#include "../communication.h"
#include <signal.h>

#define		EPOLLSIZE	1024

static void event_fun(struct comm_context* commctx, struct comm_tcp* commtcp, void* usr);

int main(int argc, char* argv[])
{
	int i = 0, k = 0, fd = 0;
	int pckidx = 0, frmidx = 0;
	int retval = -1, port = 0;
	char content[1024] = {};
	char str_port[64] = {0};

	struct cbinfo		finishedcb = {0};
	struct comm_context*	commctx = NULL;
	struct comm_message	message = {0};

	if (unlikely(argc < 4)) {
		/* 最后一个参数为绑定几个端口，输入端口为起始，后续端口都直接加1 */
		printf("usage:%s <ipaddr> <port> <bind_times>\n", argv[0]);
		return retval;
	}

	signal(SIGPIPE, SIG_IGN);
	port = atoi(argv[2]);
	message.content = content;
	commctx = comm_ctx_create(EPOLLSIZE);
	if (unlikely(!commctx)) {
		log("server comm_ctx_create failed\n");
		return retval;
	}

	/* 设置回调函数的相关信息 */
	finishedcb.callback = event_fun;
	finishedcb.usr = &message;
	for (i = 0; i < atoi(argv[3]); i++) {
		sprintf(str_port, "%d", (port+i));
		if (unlikely((fd = comm_socket(commctx, argv[1], str_port, &finishedcb, COMM_BIND)) == -1)) {
			log("server comm_socket failed\n");
			comm_ctx_destroy(commctx);
			return retval;
		}
		log("server bind successed fd:%d\n", fd);
	}

	/* 循环接收 发送数据 */
	while (1) {
		if (message.fd == -1) {
			printf("close aready destroy anything\n");
			comm_ctx_destroy(commctx);
			printf("destroy done and quit\n");
			return -1;
		}
		if (comm_recv(commctx, &message, true, -1) > -1) {
			for (pckidx = 0, k = 0; pckidx < message.package.packages; pckidx++) {
				int  size = 0;
				char buff[1024] = {};
				for (frmidx = 0; frmidx < message.package.frames_of_package[pckidx]; frmidx++, k++) {
					memcpy(&buff[size], &message.content[message.package.frame_offset[k]],message.package.frame_size[k]);
					size += message.package.frame_size[k];
				//	buff[size++] = ' ';
				}
				log("messag fd: %d message body: %s\n",message.fd, buff);
			}
			/* 接收成功之后将此消息体再返回给用户 */
			if (message.fd > 0) {
				comm_send(commctx, &message, false, -1);
			}
		} else {
			log("comm_recv failed\n");
			sleep(1);
		}

	}
	log("goint to detroy everything here\n");
	comm_ctx_destroy(commctx);
	return retval;
}

void close_fun(void *usr)
{
	//struct comm_tcp* commtcp = (struct comm_tcp*)usr;
	struct comm_message *message = (struct comm_message*)usr;
	message->fd = -1;
	printf("server here is close_fun():%d\n", message->fd);
//	printf("server here is close_fun():%d\n", commtcp->fd);
}

void write_fun(void *usr)
{
	struct comm_tcp* commtcp = (struct comm_tcp*)usr;
	printf("server here is write_fun(): %d\n", commtcp->fd);
}

void read_fun(void *usr)
{
	struct comm_tcp* commtcp = (struct comm_tcp*)usr;
	printf("server here is read_fun(): %d\n", commtcp->fd);
}

void accept_fun(void *usr)
{
	struct comm_tcp* commtcp = (struct comm_tcp*)usr;
	printf("server here is accept_fun(): %d\n", commtcp->fd);
}

void timeout_fun(void *usr)
{
	printf("server here is timeout_fun()\n");
}

static void event_fun(struct comm_context* commctx, struct comm_tcp* commtcp, void* usr)
{
	switch (commtcp->stat)
	{
		case FD_CLOSE:
			close_fun(usr);
			break;
		case FD_WRITE:
			write_fun(commtcp);
			break;
		case FD_READ:
			read_fun(commtcp);
			break;
		case FD_INIT: 
			if (commtcp->type == COMM_ACCEPT) {
				accept_fun(commtcp);
			}
		default:
			timeout_fun(commtcp);
			break;
	}
}
