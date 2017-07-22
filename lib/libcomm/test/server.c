#include <signal.h>
#include "../comm_api.h"

static void event_fun(struct comm_context *commctx, struct comm_tcp *commtcp, void *usr);

int main(int argc, char *argv[])
{
	// signal(SIGPIPE, SIG_IGN);

	if (unlikely(argc < 4)) {
		/* 最后一个参数为绑定几个端口，输入端口为起始，后续端口都直接加1 */
		printf("usage:%s <ipaddr> <port> <bind_times>\n", argv[0]);
		return -1;
	}

	char    *ipaddr = argv[1];
	int     port = atoi(argv[2]);
	int     bind_times = atoi(argv[3]);

	/*创建上下文*/
	struct comm_context *commctx = commapi_ctx_create();
	assert(commctx);

	/* 设置回调函数的相关信息 */
	struct comm_cbinfo cbinfo = { 0 };
	cbinfo.callback = event_fun;
	cbinfo.usr = NULL;

	/*创建多个监听端口*/
	int i = 0, fd = 0;

	for (i = 0; i < bind_times; i++) {
		char str_port[64] = { 0 };
		snprintf(str_port, 64, "%d", (port + i));

		if (unlikely((fd = commapi_socket(commctx, ipaddr, str_port, &cbinfo, COMM_BIND)) == -1)) {
			loger("server comm_socket failed\n");
			commapi_ctx_destroy(commctx);
			return -1;
		}

		loger("server bind successed fd:%d\n", fd);
	}

	/* 循环接收 发送数据 */
	while (1) {
		/*设置接收空间*/
		struct comm_message message = { 0 };
		commmsg_make(&message, 1024);

		int err = commapi_recv(commctx, &message);

		if (err) {
			commmsg_free(&message);
			loger("comm_recv failed\n");
			sleep(1);
			continue;
		}

		int k = 0;

		for (int pckidx = 0; pckidx < message.package.packages; pckidx++) {
			/*unpackage*/
			for (int frmidx = 0; frmidx < message.package.frames_of_package[pckidx]; frmidx++, k++) {
				printf("frame %d data :%s\n", frmidx + 1, commmsg_frame_addr(&message, k));
				printf("frame %d size :%d\n", frmidx + 1, commmsg_frame_size(&message, k));
			}
		}

		loger("\x1B[1;31m" "message fd:%d message body:%.*s socket_type:%d\n" "\x1B[m",
			message.fd, (int)message.package.raw_data.len, message.package.raw_data.str, message.ptype);

		/* 接收成功之后将此消息体再返回给用户 */
		err = commapi_send(commctx, &message);

		if (err) {
			loger("comm_recv failed\n");
		}

		commmsg_free(&message);
		sleep(1);
	}

	loger("going to detroy everything here\n");
	commapi_ctx_destroy(commctx);
	return 0;
}

void close_fun(void *usr)
{
#if 1
	struct comm_tcp *commtcp = (struct comm_tcp *)usr;
	printf("server here is close_fun():%d\n", commtcp->fd);
#else
	struct comm_message *message = (struct comm_message *)usr;
	printf("server here is close_fun():%d\n", message->fd);
	message->fd = -1;
#endif
}

void write_fun(void *usr)
{
	struct comm_tcp *commtcp = (struct comm_tcp *)usr;

	printf("server here is write_fun(): %d\n", commtcp->fd);
}

void read_fun(void *usr)
{
	struct comm_tcp *commtcp = (struct comm_tcp *)usr;

	printf("server here is read_fun(): %d\n", commtcp->fd);
}

void accept_fun(void *usr)
{
	struct comm_tcp *commtcp = (struct comm_tcp *)usr;

	printf("server here is accept_fun(): %d\n", commtcp->fd);
}

void timeout_fun(void *usr)
{
	struct comm_tcp *commtcp = (struct comm_tcp *)usr;

	printf("server here is timeout_fun(): %d\n", commtcp->fd);
}

static void event_fun(struct comm_context *commctx, struct comm_tcp *commtcp, void *usr)
{
	switch (commtcp->stat)
	{
		case FD_CLOSE:
			close_fun(commtcp);
			commapi_close(commctx, commtcp->fd);
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

