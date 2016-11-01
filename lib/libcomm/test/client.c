#include "../communication.h"
#include <signal.h>


#define TEST_FRAME_1_DATA	"here "
#define TEST_FRAME_2_DATA	"goes "
#define TEST_FRAME_3_DATA	"fd"
#define TEST_FRAME_4_DATA	"not "
#define TEST_FRAME_5_DATA	"know"

static void gain_package_data_only(struct comm_message *message, int fd)
{
	char    buff[1024] = {};
	sprintf(buff, "%s%s%s", TEST_FRAME_1_DATA, TEST_FRAME_2_DATA, TEST_FRAME_3_DATA);
	

	int     frames_of_package[1] = { 3 };			/* 每个包里面帧数 */

	int     frame_offset[3] = {
		0,
		strlen(TEST_FRAME_1_DATA),
		strlen(TEST_FRAME_1_DATA TEST_FRAME_2_DATA),
	};			/* 每帧的偏移 */

	int     frame_size[3] = {
		strlen(TEST_FRAME_1_DATA),
		strlen(TEST_FRAME_2_DATA),
		strlen(TEST_FRAME_3_DATA),
	};	/* 每帧的大小 */
	
	message->fd = fd;
	message->content = buff;
	message->config = ZIP_COMPRESSION | AES_ENCRYPTION;
	message->socket_type = REQ_METHOD;
	message->package.dsize = strlen(buff);
	message->package.frames = 3;
	message->package.packages = 1;

	int     pckidx = 0, frmidx = 0;
	int     k = 0;
	for (pckidx = 0; pckidx < message->package.packages; pckidx++) {
		for (frmidx = 0; frmidx < frames_of_package[pckidx]; frmidx++, k++) {
			message->package.frame_offset[k] = frame_offset[k];
			message->package.frame_size[k] = frame_size[k];
		}
	}

	memcpy(message->package.frames_of_package, frames_of_package, (sizeof(int)) * message->package.packages);
	return;
}

static void gain_package_data_more(struct comm_message *message, int fd)
{
	char    buff[1024] = {};
	sprintf(buff, "%s%s%s%s%s", TEST_FRAME_1_DATA, TEST_FRAME_2_DATA, TEST_FRAME_3_DATA, TEST_FRAME_4_DATA, TEST_FRAME_5_DATA);


	int     frames_of_package[2] = { 3, 2 };

	int     frame_offset[5] = {
		0,
		strlen(TEST_FRAME_1_DATA),
		strlen(TEST_FRAME_1_DATA TEST_FRAME_2_DATA TEST_FRAME_3_DATA),
		strlen(TEST_FRAME_1_DATA TEST_FRAME_2_DATA TEST_FRAME_3_DATA TEST_FRAME_4_DATA),
		strlen(TEST_FRAME_1_DATA TEST_FRAME_2_DATA TEST_FRAME_3_DATA TEST_FRAME_4_DATA TEST_FRAME_5_DATA),
	};			/* 每帧的偏移 */

	int     frame_size[5] = {
		strlen(TEST_FRAME_1_DATA),
		strlen(TEST_FRAME_2_DATA),
		strlen(TEST_FRAME_3_DATA),
		strlen(TEST_FRAME_4_DATA),
		strlen(TEST_FRAME_5_DATA),
	};	/* 每帧的大小 */

	message->fd = fd;
	message->content = buff;
	message->config = ZIP_COMPRESSION | AES_ENCRYPTION;
	message->socket_type = REQ_METHOD;
	message->package.dsize = strlen(buff);
	message->package.frames = 5;
	message->package.packages = 2;

	int     pckidx = 0, frmidx = 0;
	int     k = 0;
	for (pckidx = 0; pckidx < message->package.packages; pckidx++) {
		for (frmidx = 0; frmidx < frames_of_package[pckidx]; frmidx++, k++) {
			message->package.frame_offset[k] = frame_offset[k];
			message->package.frame_size[k] = frame_size[k];
		}
	}

	memcpy(message->package.frames_of_package, frames_of_package, (sizeof(int)) * message->package.packages);
	return;
}

static void gain_package_data_null(struct comm_message *message, int fd)
{
	message->fd = fd;
	message->content = NULL;
	message->config = ZIP_COMPRESSION | AES_ENCRYPTION;
	message->socket_type = REQ_METHOD;
	message->package.dsize = 0;
	message->package.frames = 0;
	message->package.packages = 1;

	message->package.frame_offset[0] = 0;
	message->package.frame_size[0] = 0;
	message->package.frames_of_package[0] = 0;
	return;
}

static void event_fun(struct comm_context *commctx, struct comm_tcp *commtcp, void *usr);

int main(int argc, char *argv[])
{
	signal(SIGPIPE, SIG_IGN);
	/*get args*/
	if (unlikely(argc < 4)) {
		/* 最后一个参数为绑定几个端口，输入端口为起始，后续端口都直接加1 */
		printf("usage:%s <ipaddr> <port> <connect_times>\n", argv[0]);
		return -1;
	}
	char *host = argv[1];
	char *port = argv[2];
	int numb = atoi(argv[3]);


	/*create ctx*/
	struct comm_context     *commctx = comm_ctx_create(-1);
	if (unlikely(!commctx)) {
		loger("client comm_ctx_create failed\n");
		return -1;
	}

	/* 设置回调函数的相关信息 */
	int n = 0;
	int fds[numb] = {};
	for (; n < numb; n++) {
		struct cbinfo           finishedcb = { 0 };
		finishedcb.callback = event_fun;
		finishedcb.usr = NULL;
		if (unlikely((fds[n] = comm_socket(commctx, host, port, &finishedcb, COMM_CONNECT | CONNECT_ANYWAY)) == -1)) {
			comm_ctx_destroy(commctx);
			loger("client comm_socket failed\n");
			return -1;
		}
		loger("client connect successed fd:%d\n",fds[n]);
	}
	loger("\x1B[1;31m""client connect over!\n""\x1B[m");

	/* 循环接收 发送数据 */
	struct comm_message     sendmsg = { 0 };
	struct comm_message     recvmsg = { 0 };
	char*	content = NULL;
	int	datasize = 1024*1024*100;
	NewArray(content, datasize);
	recvmsg.content = content;

	while (1) {
		for (n = 0; n < numb; n++) {
			gain_package_data_only(&sendmsg, fds[n]);
			
			if (comm_send(commctx, &sendmsg, false, -1) <= -1) {
				loger("comm_send failed\n");
				continue;
			}
			loger("\x1B[1;34m""fd:%d send data successed\n""\x1B[m", fds[n]);
			
			if (comm_recv(commctx, &recvmsg, true, -1) <= -1) {
				loger("comm_recv failed\n");
				continue;
			}
			loger("\x1B[1;34m""fd:%d recv data successed\n""\x1B[m", fds[n]);

			int     pckidx = 0, frmidx = 0;
			int     k = 0;
			for (pckidx = 0; pckidx < recvmsg.package.packages; pckidx++) {
				for (frmidx = 0; frmidx < recvmsg.package.frames_of_package[pckidx]; frmidx++, k++) {
					printf("frame %d data :%s\n", frmidx + 1, &recvmsg.content[recvmsg.package.frame_offset[k]]);
					printf("frame %d size :%d\n", frmidx + 1, recvmsg.package.frame_size[k]);
				}

				loger("\x1B[1;31m""message fd:%d message body:%.*s socket_type:%d\n""\x1B[m", recvmsg.fd, recvmsg.package.dsize, recvmsg.content, recvmsg.socket_type);
			}
		}
	}

	/*over*/
	loger("goint to detroy everything here\n");
	comm_ctx_destroy(commctx);
	return -1;
}

void close_fun(void *usr)
{
	struct comm_tcp *commtcp = (struct comm_tcp *)usr;

	printf("client here is close_fun():%d\n", commtcp->fd);
}

void write_fun(void *usr)
{
	struct comm_tcp *commtcp = (struct comm_tcp *)usr;

	printf("client here is write_fun(): %d\n", commtcp->fd);
}

void read_fun(void *usr)
{
	struct comm_tcp *commtcp = (struct comm_tcp *)usr;

	printf("client here is read_fun(): %d\n", commtcp->fd);
}

void accept_fun(void *usr)
{
	struct comm_tcp *commtcp = (struct comm_tcp *)usr;

	printf("client here is accept_fun(): %d\n", commtcp->fd);
}

void timeout_fun(void *usr)
{
	printf("client here is timeout_fun()\n");
}

static void event_fun(struct comm_context *commctx, struct comm_tcp *commtcp, void *usr)
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

