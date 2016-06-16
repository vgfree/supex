#include "cidmap.h"
#include "simulate.h"

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

void send_message(char *str, int flag);

void *pull_thread(void *usr)
{
	void    *server_simulator = zmq_socket(g_ctx, ZMQ_PULL);
	int     rc = zmq_connect(server_simulator, "tcp://127.0.0.1:8092");

	assert(rc == 0);

	while (1) {
		int     more;
		size_t  more_size = sizeof(more);
		int     frames = 0;
		char    *test[20] = {};
		do {
			zmq_msg_t       part;
			int             rc = zmq_msg_init(&part);
			assert(rc == 0);
			rc = zmq_recvmsg(server_simulator, &part, 0);
			assert(rc != -1);
			test[frames] = (char *)malloc((zmq_msg_size(&part) + 1) * sizeof(char));
			memcpy(test[frames], zmq_msg_data(&part), zmq_msg_size(&part));
			test[frames][zmq_msg_size(&part)] = '\0';
			printf("recv data:%s\n", test[frames]);
			frames++;
			zmq_getsockopt(server_simulator, ZMQ_RCVMORE, &more, &more_size);
			zmq_msg_close(&part);
		} while (more);
		// uid
		char *cid = get_first_cid();

		while (cid) {
			char    downstream[20] = "downstream\0";
			char    cid_str[10] = "cid\0";
			send_message(downstream, ZMQ_SNDMORE);
			send_message(cid_str, ZMQ_SNDMORE);
			send_message(cid, ZMQ_SNDMORE);
			printf("send cid:%s\n", cid);
			free(cid);

			for (int i = 0; i < frames - 1; i++) {
				send_message(test[i], ZMQ_SNDMORE);
			}

			printf("the last test[i]:%s.", test[frames - 1]);
			char buf[5000] = {};
			snprintf(buf, 30, "当前在线人数：%d.\n", get_numbers());
			strcat(buf, "cid msg,");
			strcat(buf, test[frames - 1]);
			send_message(buf, 0);
			cid = get_next_cid();
		}

		cid = get_first_cid();

		while (cid) {
			char    downstream[20] = "downstream\0";
			char    uid_str[10] = "uid\0";
			send_message(downstream, ZMQ_SNDMORE);
			send_message(uid_str, ZMQ_SNDMORE);
			char uid_buf[30] = "uid";
			strcat(uid_buf, cid);
			send_message(uid_buf, ZMQ_SNDMORE);
			printf("send cid:%s\n", cid);
			free(cid);

			for (int i = 0; i < frames - 1; i++) {
				send_message(test[i], ZMQ_SNDMORE);
			}

			char buf[5000] = "uid msg,";
			strcat(buf, test[frames - 1]);
			send_message(buf, 0);
			cid = get_next_cid();
		}

		// gid
		send_message("downstream", ZMQ_SNDMORE);
		send_message("gid", ZMQ_SNDMORE);
		send_message("gid0", ZMQ_SNDMORE);

		for (int i = 0; i < frames - 1; i++) {
			send_message(test[i], ZMQ_SNDMORE);
		}

		char buf[5000] = "gid msg,";
		strcat(buf, test[frames - 1]);
		send_message(buf, 0);

		for (int i = 0; i < frames; i++) {
			free(test[i]);
		}
	}

	zmq_close(server_simulator);
}

static void *push_server = NULL;
void init_push_server()
{
	push_server = zmq_socket(g_ctx, ZMQ_PUSH);
	int rc = zmq_connect(push_server, "tcp://127.0.0.1:8090");
	assert(rc == 0);
	printf("connect push server success.");
}

void send_message(char *str, int flag)
{
	zmq_msg_t       part;
	int             rc = zmq_msg_init_size(&part, strlen(str));

	assert(rc == 0);
	memcpy(zmq_msg_data(&part), str, strlen(str));
	zmq_sendmsg(push_server, &part, flag);
}

void destroy_push_server()
{
	zmq_close(push_server);
}

