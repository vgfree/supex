#include <signal.h>
#include "../comm_api.h"

#define TEST_FRAME_1_DATA       "here "
#define TEST_FRAME_2_DATA       "goes "
#define TEST_FRAME_3_DATA       "fd"
#define TEST_FRAME_4_DATA       "not "
#define TEST_FRAME_5_DATA       "know"

static void gain_package_data_only(struct comm_message *message, int fd)
{
	char buff[1024] = {};

	sprintf(buff, "%s%s%s", TEST_FRAME_1_DATA, TEST_FRAME_2_DATA, TEST_FRAME_3_DATA);

	int frames_of_package[1] = { 3 };			/* 每个包里面帧数 */

	int frame_offset[3] = {
		0,
		strlen(TEST_FRAME_1_DATA),
		strlen(TEST_FRAME_1_DATA TEST_FRAME_2_DATA),
	};			/* 每帧的偏移 */

	int frame_size[3] = {
		strlen(TEST_FRAME_1_DATA),
		strlen(TEST_FRAME_2_DATA),
		strlen(TEST_FRAME_3_DATA),
	};	/* 每帧的大小 */

	message->fd = fd;
	message->flags = ZIP_COMPRESSION | AES_ENCRYPTION;
	message->ptype = REQ_METHOD;
	message->package.frames = 3;
	message->package.packages = 1;
	commsds_push_tail(&message->package.raw_data, buff, strlen(buff));

	int     pckidx = 0, frmidx = 0;
	int     k = 0;

	for (pckidx = 0; pckidx < message->package.packages; pckidx++) {
		for (frmidx = 0; frmidx < frames_of_package[pckidx]; frmidx++, k++) {
			message->package.frame_offset[k] = frame_offset[k];
			message->package.frame_size[k] = frame_size[k];
		}
	}

	memcpy(message->package.frames_of_package, frames_of_package, (sizeof(int)) * message->package.packages);
}

static void gain_package_data_more(struct comm_message *message, int fd)
{
	char buff[1024] = {};

	sprintf(buff, "%s%s%s%s%s", TEST_FRAME_1_DATA, TEST_FRAME_2_DATA, TEST_FRAME_3_DATA, TEST_FRAME_4_DATA, TEST_FRAME_5_DATA);

	int frames_of_package[2] = { 3, 2 };

	int frame_offset[5] = {
		0,
		strlen(TEST_FRAME_1_DATA),
		strlen(TEST_FRAME_1_DATA TEST_FRAME_2_DATA TEST_FRAME_3_DATA),
		strlen(TEST_FRAME_1_DATA TEST_FRAME_2_DATA TEST_FRAME_3_DATA TEST_FRAME_4_DATA),
		strlen(TEST_FRAME_1_DATA TEST_FRAME_2_DATA TEST_FRAME_3_DATA TEST_FRAME_4_DATA TEST_FRAME_5_DATA),
	};			/* 每帧的偏移 */

	int frame_size[5] = {
		strlen(TEST_FRAME_1_DATA),
		strlen(TEST_FRAME_2_DATA),
		strlen(TEST_FRAME_3_DATA),
		strlen(TEST_FRAME_4_DATA),
		strlen(TEST_FRAME_5_DATA),
	};	/* 每帧的大小 */

	message->fd = fd;
	message->flags = ZIP_COMPRESSION | AES_ENCRYPTION;
	message->ptype = REQ_METHOD;
	message->package.frames = 5;
	message->package.packages = 2;
	commsds_push_tail(&message->package.raw_data, buff, strlen(buff));

	int     pckidx = 0, frmidx = 0;
	int     k = 0;

	for (pckidx = 0; pckidx < message->package.packages; pckidx++) {
		for (frmidx = 0; frmidx < frames_of_package[pckidx]; frmidx++, k++) {
			message->package.frame_offset[k] = frame_offset[k];
			message->package.frame_size[k] = frame_size[k];
		}
	}

	memcpy(message->package.frames_of_package, frames_of_package, (sizeof(int)) * message->package.packages);
}

static void gain_package_data_null(struct comm_message *message, int fd)
{
	message->fd = fd;
	message->flags = ZIP_COMPRESSION | AES_ENCRYPTION;
	message->ptype = REQ_METHOD;
	message->package.frames = 0;
	message->package.packages = 1;

	message->package.frame_offset[0] = 0;
	message->package.frame_size[0] = 0;
	message->package.frames_of_package[0] = 0;
}

static void event_fun(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	switch (step)
	{
		case STEP_INIT:
			printf("client here is connect : %d\n", socket);
			break;

		case STEP_ERRO:
			printf("client here is error : %d\n", socket);
			break;

		case STEP_WAIT:
			printf("client here is wait : %d\n", socket);
			break;


		case STEP_STOP:
			printf("client here is close : %d\n", socket);
			break;

		default:
			printf("unknow!\n");
			break;
	}
}



int main(int argc, char *argv[])
{
	// signal(SIGPIPE, SIG_IGN);

	/*get args*/
	if (unlikely(argc < 4)) {
		/* 最后一个参数为绑定几个端口，输入端口为起始，后续端口都直接加1 */
		printf("usage:%s <ipaddr> <port> <connect_times>\n", argv[0]);
		return -1;
	}

	char    *ipaddr = argv[1];
	int     port = atoi(argv[2]);
	int     bind_times = atoi(argv[3]);

	/*create ctx*/
	struct comm_context *commctx = commapi_ctx_create();

	if (unlikely(!commctx)) {
		loger("client comm_ctx_create failed\n");
		return -1;
	}

	/* 设置回调函数的相关信息 */
	struct comm_cbinfo cbinfo = { 0 };
	cbinfo.fcb = event_fun;
	cbinfo.usr = NULL;

	int     i = 0;
	int     fds[bind_times];

	for (; i < bind_times; i++) {
		char str_port[64] = { 0 };
		snprintf(str_port, 64, "%d", (port + i));

		if (unlikely((fds[i] = commapi_socket(commctx, ipaddr, str_port, &cbinfo, COMM_CONNECT)) == -1)) {
			commapi_ctx_destroy(commctx);
			loger("client comm_socket failed\n");
			return -1;
		}

		loger("client connect successed fd:%d\n", fds[i]);
	}

	loger("\x1B[1;31m" "client connect over!\n" "\x1B[m");

	/* 循环接收 发送数据 */
	struct comm_message     sendmsg = { 0 };
	struct comm_message     recvmsg = { 0 };

	while (1) {
		for (i = 0; i < bind_times; i++) {
			commmsg_make(&sendmsg, 1024);
			sendmsg.fd = port + i;
			gain_package_data_only(&sendmsg, fds[i]);

			int err = commapi_send(commctx, &sendmsg);
			commmsg_free(&sendmsg);

			if (err) {
				loger("comm_send failed\n");
				continue;
			}

			loger("\x1B[1;34m" "fd:%d send data successed\n" "\x1B[m", fds[i]);

			commmsg_make(&recvmsg, 1024);
			err = commapi_recv(commctx, &recvmsg);

			if (err) {
				loger("comm_recv failed\n");
				commmsg_free(&recvmsg);
				continue;
			}

			loger("\x1B[1;34m" "fd:%d recv data successed\n" "\x1B[m", fds[i]);

			int     pckidx = 0, frmidx = 0;
			int     k = 0;

			for (pckidx = 0; pckidx < recvmsg.package.packages; pckidx++) {
				for (frmidx = 0; frmidx < recvmsg.package.frames_of_package[pckidx]; frmidx++, k++) {
					printf("frame %d data :%s\n", frmidx + 1, commmsg_frame_addr(&recvmsg, k));
					printf("frame %d size :%d\n", frmidx + 1, commmsg_frame_size(&recvmsg, k));
				}
			}

			loger("\x1B[1;31m" "message fd:%d message body:%.*s socket_type:%d\n" "\x1B[m",
				recvmsg.fd, (int)recvmsg.package.raw_data.len, recvmsg.package.raw_data.str, recvmsg.ptype);
			commmsg_free(&recvmsg);
		}
	}

	/*over*/
	loger("goint to detroy everything here\n");
	commapi_ctx_destroy(commctx);
	return -1;
}




