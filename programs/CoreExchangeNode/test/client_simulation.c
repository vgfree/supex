#include "communication.h"
#include "comm_message_operator.h"
#include "core_exchange_node_test.h"
#include "sys/time.h"
//#include "loger.h"

#include <pthread.h>

void print_current_time() {
	struct timeval tv;
//	struct timezone tz;
//	gettimeofday(&tv, &tz);
	printf("tv_sec:%d", tv.tv_sec);
	printf("tv_usec:%d--", tv.tv_usec);
}

static int connectfd = 0;
struct comm_context *g_ctx = NULL;
void *client_thread_read(void *usr) {
	while(1) {
		struct comm_message msg = {};
		init_msg(&msg);
		printf("start recv msg.\n");
		comm_recv(g_ctx, &msg, true, -1);
		int size = 0;
		char *frame = get_msg_frame(0, &msg, &size);
		char *buf = (char *)malloc((size + 1) * sizeof(char));
		int i;
		for (i = 0; i < size; i++) {
			printf("%x,", frame[i]);
		}
		memcpy(buf, frame, size);
		buf[size] = '\0';
		printf("recv msg:");
		print_current_time();
		printf("%s\n", buf);
		free(buf);
		destroy_msg(&msg);
	}
	return NULL;
}

int test_simulate_client(char *ip) {
	g_ctx = comm_ctx_create(EPOLL_SIZE);
	struct cbinfo callback_info = {};
	callback_info.callback = NULL;
	printf("ip:%s\n", ip);
	connectfd = comm_socket(g_ctx, ip, "8082", &callback_info, COMM_CONNECT);
	printf("connectfd:%d\n", connectfd);
	if(connectfd == -1) {
		printf("connect error.");
		return -1;
	}
	pthread_t tid;
	assert(pthread_create(&tid, NULL, client_thread_read, NULL) == 0);
	char str[1024];
	snprintf(str, 10, "tid:%u", tid);
	while (fgets(str, 1024, stdin) != NULL) {
		printf("send msg:");
		print_current_time();
		printf("\n");
		struct comm_message msg = {};
		init_msg(&msg);
		msg.socket_type = PAIR_METHOD;
		set_msg_fd(&msg, connectfd);
		set_msg_frame(0, &msg, strlen(str), str);
		comm_send(g_ctx, &msg, true, -1);
		destroy_msg(&msg);
//		sleep(5);
	}
	return 0;
}
