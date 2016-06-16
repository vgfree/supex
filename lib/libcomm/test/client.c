#include "../communication.h"
#include <signal.h>

#define		EPOLLSIZE	1024

static int send_data(struct comm_context *commctx, struct comm_message *message, int fd);

static void event_fun(struct comm_context* commctx, struct comm_tcp* commtcp, void* usr);

int main(int argc, char* argv[])
{
	int n = 0, k = 0, fd[10] = {};
	int pckidx = 0, frmidx = 0;
	int retval = -1, port = 0;
	char content[1024] = {};
	char str_port[64] = {0};

	struct cbinfo		finishedcb = {0};
	struct comm_context*	commctx = NULL;
	struct comm_message	sendmsg = {0};
	struct comm_message	recvmsg = {0};

	if (unlikely(argc < 4)) {
		/* 最后一个参数为绑定几个端口，输入端口为起始，后续端口都直接加1 */
		printf("usage:%s <ipaddr> <port> <connect_times>\n", argv[0]);
		return retval;
	}

	signal(SIGPIPE, SIG_IGN);
	port = atoi(argv[2]);
	recvmsg.content = content;
	commctx = comm_ctx_create(EPOLLSIZE);
	if (unlikely(!commctx)) {
		log("client comm_ctx_create failed\n");
		return retval;
	}

	/* 设置回调函数的相关信息 */
	finishedcb.callback = event_fun;
	//finishedcb.usr = &message;
	for (n = 0; n < atoi(argv[3]); n++) {
		sprintf(str_port, "%d", (port+n));
		if (unlikely((fd[n] = comm_socket(commctx, argv[1], str_port, &finishedcb, COMM_CONNECT | CONNECT_ANYWAY)) == -1)) {
			comm_ctx_destroy(commctx);
			log("client comm_socket failed\n");
			return retval;
		}
		log("client connect successed fd:%d\n", fd[n]);
	}

	/* 循环接收 发送数据 */
	while (1) {
		int i = 0;
		for (i = 0; i < n; i++) {
			if (send_data(commctx, &sendmsg, fd[i]) > -1) {
				log("comm_send successed\n");
				if (comm_recv(commctx, &recvmsg, false, -1) > -1) {
					for (pckidx = 0, k = 0; pckidx < recvmsg.package.packages; pckidx++) {
						int  size = 0;
						char buff[1024] = {};
						for (frmidx = 0; frmidx < recvmsg.package.frames_of_package[pckidx]; frmidx++, k++) {
							memcpy(&buff[size], &recvmsg.content[recvmsg.package.frame_offset[k]], recvmsg.package.frame_size[k]);
							size += recvmsg.package.frame_size[k];
						}
						log("messag fd: %d message body: %s\n",recvmsg.fd, buff);
					}
				} else {
					log("comm_recv failed\n");
				}
			} else {
				log("comm_send failed\n");
			}
			sleep(1);
		}
	}
	log("goint to detroy everything here\n");
	comm_ctx_destroy(commctx);
	return retval;
}

void close_fun(void *usr)
{
	struct comm_tcp* commtcp = (struct comm_tcp*)usr;
	printf("client here is close_fun():%d\n", commtcp->fd);
}

void write_fun(void *usr)
{
	struct comm_tcp* commtcp = (struct comm_tcp*)usr;
	printf("client here is write_fun(): %d\n", commtcp->fd);
}

void read_fun(void *usr)
{
	struct comm_tcp* commtcp = (struct comm_tcp*)usr;
	printf("client here is read_fun(): %d\n", commtcp->fd);
}

void accept_fun(void *usr)
{
	struct comm_tcp* commtcp = (struct comm_tcp*)usr;
	printf("client here is accept_fun(): %d\n", commtcp->fd);
}

void timeout_fun(void *usr)
{
	printf("client here is timeout_fun()\n");
}

static void event_fun(struct comm_context* commctx, struct comm_tcp* commtcp, void* usr)
{
	switch (commtcp->stat)
	{
		case FD_CLOSE:
			close_fun(commtcp);
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

static int send_data(struct comm_context *commctx, struct comm_message *message, int fd)
{
	int i = 0, j = 0, k = 0;
	int pckidx = 0, frmidx = 0;
	int retval = -1; 
	char buff[1024] =  {};
	int frames_of_package[1] = {4};				/* 每个包里面帧数 */
	sprintf(buff, "%s%d", "here goes fd:", fd);

	int frame_offset[4] = {0, 
					strlen("here "),
					strlen("here goes "),
					strlen("here goes fd:")
				};						/* 每帧的偏移 */
	int frame_size[4] = {		strlen("here "), 
					strlen("goes "), 
					strlen("fd:"),
					strlen(buff) - strlen("here goes fd:")
				};						/* 每帧的大小 */
	message->fd = fd;
	message->content = buff;
	message->config = ZIP_COMPRESSION | AES_ENCRYPTION;
	message->socket_type = REQ_METHOD;
	message->package.dsize = strlen(buff);
	message->package.frames = 4;
	message->package.packages = 1;

	for (pckidx = 0, k = 0 ; pckidx < message->package.packages; pckidx++) {
		for (frmidx = 0; frmidx < frames_of_package[pckidx]; frmidx++, k++) {
			message->package.frame_offset[k] = frame_offset[k];
			message->package.frame_size[k] = frame_size[k];
		}
	} 
	memcpy(message->package.frames_of_package, frames_of_package, (sizeof(int))*message->package.packages);
	comm_send(commctx, message, false, -1);

	char buff1[1024] = "nothing matters I'm on my own";
	int frames_of_package1[2] = {2, 4};
	int frame_offset1[6] = {0,
					strlen("nothing "),
					strlen("nothing matters "),
					strlen("nothing matters I'm "),
					strlen("nothing matters I'm on "),
					strlen("nothing matters I'm on my ")
				};
	int frame_size1[6] = {
					strlen("nothing "),
					strlen("matters "),
					strlen("I'm "),
					strlen("on "),
					strlen("my "),
					strlen("own")
				};
	/* 进行第二次发送数据 */
	message->content = buff1;
	message->package.packages = 2;
	message->package.frames = 6;
	message->package.dsize = strlen(buff1);
	for (pckidx = 0, k = 0; pckidx < message->package.packages; pckidx++) {
		for (frmidx = 0; frmidx < frames_of_package1[pckidx]; frmidx++, k++) {
			message->package.frame_offset[k] = frame_offset1[k];
			message->package.frame_size[k] = frame_size1[k];
		}
	} 
	memcpy(message->package.frames_of_package, frames_of_package1, (sizeof(int))*message->package.packages);
	return comm_send(commctx, message, false, -1);
}
