#include "comm_message_operator.h"
#include "sys/time.h"
#include "communication.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "loger.h"

static struct comm_context *comm_ctx = NULL;

void print_current_time()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	printf("tv_sec:%d", tv.tv_sec);
	printf("tv_usec:%d--", tv.tv_usec);
}

static void event_fun(struct comm_context *commctx, struct comm_tcp *commtcp, void *usr);

static int send_data(struct comm_context *commctx, int fd)
{
	int                     i = 0;
	int                     datasize = 10;
	char                    content[1024] = {};
	struct comm_message     sendmsg = {};

	sendmsg.fd = fd;
	sendmsg.config = 0;
	sendmsg.socket_type = -1;
	sendmsg.content = content;
	sendmsg.package.packages = 1;
	sendmsg.package.frames = 1;
	sendmsg.package.dsize = datasize;
	sendmsg.package.frame_size[0] = datasize;
	sendmsg.package.frame_offset[0] = 0;
	sendmsg.package.frames_of_package[0] = 1;

	for (i = 0; i < datasize; i++) {
		sendmsg.content[i] = 'a';
	}

	comm_send(commctx, &sendmsg, false, -1);
	return 0;
}

void *read_message(void *arg)
{
	char                    content[1024] = {};
	int                     datasize = 1024;
	struct comm_message     recvmsg = {};

	while (1) {
		memset(&recvmsg, 0, sizeof(recvmsg));
		recvmsg.content = content;
		printf("\x1B[1;32m" "start recv msg.\n" "\x1B[m");
		comm_recv(comm_ctx, &recvmsg, true, -1);
		printf("\x1B[1;32m" "recv data successed fd:%d\n" "\x1B[m", recvmsg.fd);
		printf("\x1Bp1;32m" "recv_data:%.*s\n" "\x1B[m", recvmsg.package.dsize, recvmsg.content);
	}

	return NULL;
}

struct CSLog *g_imlog = NULL;

int main(int argc, char *argv[])
{
	g_imlog = CSLog_create("fdaf", 1);
	int                     i = 0, datasize = 10;
	int                     fd[10000] = {};
	char                    content[1024] = {};
	struct cbinfo           finishedcb = { 0 };
	struct comm_message     sendmsg = {};

	sendmsg.content = content;
	finishedcb.callback = event_fun;
	finishedcb.usr = &sendmsg;

	if (argc < 3) {
		printf("usge:%s <ip> <port> <client_count>\n", argv[0]);
		return -1;
	}

	comm_ctx = comm_ctx_create(-1);

	if (unlikely(!comm_ctx)) {
		printf("context create failed\n");
		return -1;
	}

	for (i = 0; i < atoi(argv[3]); i++) {
		fd[i] = comm_socket(comm_ctx, argv[1], argv[2], NULL, COMM_CONNECT);

		if (fd[i] == -1) {
			printf("establish connection failed\n");
			comm_ctx_destroy(comm_ctx);
			return -1;
		}

		printf("\x1B[1;31m" "client[%d]-fd[%d] establish connection successful!\n" "\x1B[m", i + 1, fd[i]);
	}

	pthread_t tid;
	assert(pthread_create(&tid, NULL, read_message, NULL) == 0);
	sleep(1);

	sendmsg.config = 0;
	sendmsg.socket_type = -1;
	sendmsg.content = content;
	sendmsg.package.packages = 1;
	sendmsg.package.frames = 1;
	sendmsg.package.dsize = datasize;
	sendmsg.package.frame_size[0] = datasize;
	sendmsg.package.frame_offset[0] = 0;
	sendmsg.package.frames_of_package[0] = 1;

	for (i = 0; i < datasize; i++) {
		sendmsg.content[i] = 'a';
	}

	while (1) {
		for (i = 0; i < atoi(argv[3]); i++) {
			sendmsg.fd = fd[i];

			if (comm_send(comm_ctx, &sendmsg, false, -1) == -1) {
				comm_ctx_destroy(comm_ctx);
				CSLog_destroy(g_imlog);
				return 0;
			}

			printf("\x1B[1;34m" "send data successed fd:%d\n" "\x1B[m", fd[i]);
			//			sleep(1);
		}
	}

	comm_ctx_destroy(comm_ctx);
	CSLog_destroy(g_imlog);
	return 0;
}

void close_fun(void *usr)
{
	struct comm_message *message = (struct comm_message *)usr;

	printf("client here is close_fun():%d\n", message->fd);
	message->fd = -1;
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

