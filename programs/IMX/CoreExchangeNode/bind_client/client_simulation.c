#include "communication.h"
#include "comm_message_operator.h"
#include "core_exchange_node_test.h"
#include "sys/time.h"
#define TEST 0
#include <pthread.h>
#include <uuid/uuid.h>
#include "json.h"

void print_current_time()
{
	struct timeval tv;

	//	struct timezone tz;
	//	freopen("out.txt", "w", stdout);
	gettimeofday(&tv, NULL);
	printf("tv_sec:%d", tv.tv_sec);
	printf("tv_usec:%d--", tv.tv_usec);
}

static int              connectfd = 0;
struct comm_context     *g_ctx = NULL;
void *client_thread_read(void *usr)
{
	while (1) {
		struct comm_message msg = {};
		commmsg_make(&msg, DEFAULT_MSG_SIZE);
		printf("start recv msg.\n");
		commapi_recv(g_ctx, &msg);
		int     size = 0;
		char    *frame = commmsg_frame_get(&msg, 0, &size);
		char    *buf = (char *)malloc((size + 1) * sizeof(char));
		memcpy(buf, frame, size);
		buf[size] = '\0';
		printf("recv msg:");
		print_current_time();
		printf("\033[1;31m" "\nrecv_data:%s\n" "\033[0m", buf);
		printf("\033[1;31m" "recv_data_len:%d\n" "\033[0m", strlen(buf));

		if (memcmp(buf, "bind", 4) == 0) {
			printf("recv frames : %d\n", get_max_msg_frame(&msg));
			remove_first_nframe(get_max_msg_frame(&msg), &msg);
			set_msg_frame(0, &msg, strlen(buf), buf);
			struct json_object *my_json = NULL;
			my_json = json_object_new_object();

			if (my_json == NULL) {
				printf("new json object failed.\n");
				return;
			}

			char    uuid[36];
			uuid_t  uu;
			uuid_generate(uu);
			uuid_unparse(uu, uuid);
			printf("\033[1;31m" "send uuid: %s\n" "\033[0m", uuid);
			json_object_object_add(my_json, "uid", json_object_new_string(uuid));
			memset(buf, 0, sizeof(buf));
			buf = json_object_to_json_string(my_json);
			set_msg_frame(1, &msg, strlen(buf), buf);
			printf("dsize:%d frame_size1:%d frame_size2:%d\n", msg.package.dsize, msg.package.frame_size[0], msg.package.frame_size[1]);

			if (commapi_send(g_ctx, &msg) > 0) {
				printf("\033[1;31m" "send json successfull!\n" "\033[0m");
			}

			free(buf);
			remove_first_nframe(get_max_msg_frame(&msg), &msg);
			commmsg_free(&msg);
		}
	}

	return NULL;
}

int test_simulate_client(char *ip)
{
	g_ctx = comm_ctx_create(EPOLL_SIZE);
	struct cbinfo callback_info = {};
	callback_info.callback = NULL;
	printf("\033[1;38m" "ip:%s\n" "\033[0m", ip);
	connectfd = comm_socket(g_ctx, ip, "8082", &callback_info, COMM_CONNECT);
	printf("connectfd:%d\n", connectfd);

	if (connectfd == -1) {
		printf("\033[1;31;40m connect error.\n \033[0m");
		return -1;
	}

	pthread_t tid;
	assert(pthread_create(&tid, NULL, client_thread_read, NULL) == 0);
	int     datasize = (1024 * 1024 * 2 + 1);
	char    *str = (char *)malloc(datasize);
	memset(str, 0, datasize);
	snprintf(str, 10, "tid:%u", tid);
	char temp[1024] = { 0 };

	while (fgets(str, 1024, stdin) != NULL) {
#if 0
		int i;

		for (i = 0; i < 2 * 1024 * 1024; i++) {
			str[i] = 'a';
		}
		str[i] = '\0';
#endif

		printf("\033[1;32;32m" "data_length:%d\n" "\033[0m", strlen(str));
		printf("\033[1;32;32m" "send msg:");
		print_current_time();
		printf("\n" "\033[0m");
		struct comm_message msg = {};
		commmsg_make(&msg, DEFAULT_MSG_SIZE);
		commmsg_sets(&msg, connectfd, 0, PUSH_METHOD);
		set_msg_frame(0, &msg, strlen(str), str);
		printf("\033[1;32;32m" "fgets input frames: %d\n" "\033[0m", msg.package.frames);
		commapi_send(g_ctx, &msg);
		commmsg_free(&msg);
		//		sleep(10);
	}

	return 0;
}

