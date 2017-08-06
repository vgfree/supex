#include "comm_message_operator.h"
#include "sys/time.h"
#include "communication.h"

static struct comm_context *comm_ctx = NULL;

void print_current_time()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	printf("tv_sec:%d", tv.tv_sec);
	printf("tv_usec:%d--", tv.tv_usec);
}

static int send_data(struct comm_context *commctx, int fd)
{
	int     i;
	int     datasize = 1024;
	char    *str = (char *)malloc(datasize);

	memset(str, 0, datasize);

	// fgets(str, 1024, stdin);
	for (i = 0; i < 10; i++) {
		str[i] = 's';
	}

	str[i] = '\0';
	struct comm_message sendmsg = {};
	commmsg_make(&sendmsg, DEFAULT_MSG_SIZE);
	commmsg_sets(&sendmsg, fd, 0, PUSH_METHOD);
	set_msg_frame(0, &sendmsg, strlen(str), str);
	// printf("data(send):%s, data size:%d\n", sendmsg.content, sendmsg.package.dsize);
	commapi_send(commctx, &sendmsg);
	commmsg_free(&sendmsg);
	free(str);
	str = NULL;
	return 0;
}

void *read_message(void *arg)
{
	struct comm_message recvmsg = {};

	while (1) {
		printf("\x1B[1;32m" "start recv msg.\n" "\x1B[m");
		commmsg_make(&recvmsg, DEFAULT_MSG_SIZE);
		remove_first_nframe(get_max_msg_frame(&recvmsg), &recvmsg);
		commapi_recv(comm_ctx, &recvmsg);
		size_t  size = 0;
		char    *frame = commmsg_frame_get(&recvmsg, 0, &size);
		char    *buf = (char *)malloc((size + 1) * sizeof(char));
		memcpy(buf, frame, size);
		buf[size] = '\0';
		printf("\x1Bp1;32m" "recv_data:%*.s\n" "\x1B[m", recvmsg.package.dsize, recvmsg.content);
		printf("\x1B[1;32m" "recv data successed fd:%d\n" "\x1B[m", recvmsg.fd);
		printf("\x1B[1;32m" "recv msg:" "\x1B[m");
		print_current_time();
		printf("\x1B[1;32m" "%s\n" "\x1B[m", buf);
		free(buf);
	}

	commmsg_free(&recvmsg);
	return NULL;
}

int main(int argc, char *argv[])
{
	int     fd[10000] = {};
	int     i;

	struct cbinfo cb = {};

	if (argc < 3) {
		printf("usge:%s <ip> <port> <client_count>\n", argv[0]);
		return -1;
	}

	comm_ctx = comm_ctx_create(EPOLL_SIZE);

	if (unlikely(!comm_ctx)) {
		printf("send context create failed\n");
		return -1;
	}

	for (i = 0; i < atoi(argv[3]); i++) {
		fd[i] = comm_socket(comm_ctx, argv[1], argv[2], &cb, COMM_CONNECT);

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

	while (1) {
		for (i = 0; i < atoi(argv[3]); i++) {
			send_data(comm_ctx, fd[i]);
			printf("\x1B[1;34m" "send data successed fd:%d\n" "\x1B[m", fd[i]);
			//			sleep(1);
		}
	}

	comm_ctx_destroy(comm_ctx);
	return 0;
}

