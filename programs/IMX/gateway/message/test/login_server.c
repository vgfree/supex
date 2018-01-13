#include "cidmap.h"
#include "simulate.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zmq.h>
#include <malloc.h>

void *login_thread(void *usr)
{
	void    *pull = zmq_socket(g_ctx, ZMQ_PULL);
	int     rc = zmq_connect(pull, "tcp://127.0.0.1:8101");

	assert(rc == 0);
	printf("login pull.\n");

	while (1) {
		int     more;
		size_t  more_size = sizeof(more);
		char    *test[20] = {};
		int     frames = 0;
		do {
			zmq_msg_t       part;
			int             rc = zmq_msg_init(&part);
			assert(rc == 0);
			rc = zmq_recvmsg(pull, &part, 0);
			assert(rc != -1);
			test[frames] = (char *)malloc((zmq_msg_size(&part) + 1) * sizeof(char));
			memset(test[frames], 0, zmq_msg_size(&part) + 1);
			memcpy(test[frames], zmq_msg_data(&part), zmq_msg_size(&part));
			printf("loginserver data:%s.\n", test[frames]);
			frames++;
			zmq_getsockopt(pull, ZMQ_RCVMORE, &more, &more_size);
			zmq_msg_close(&part);
		} while (more);

		if (strcmp(test[0], "status") != 0) {
			for (int i = 0; i < frames; i++) {
				free(test[i]);
			}

			continue;
		}

		if (strcmp(test[1], "connected") == 0) {
			append_cid(test[2], strlen(test[2]));
			send_to_api("setting", ZMQ_SNDMORE);
			send_to_api("gidmap", ZMQ_SNDMORE);
			send_to_api(test[2], ZMQ_SNDMORE);
			send_to_api("gid0", 0);
			send_to_api("setting", ZMQ_SNDMORE);
			send_to_api("uidmap", ZMQ_SNDMORE);
			send_to_api(test[2], ZMQ_SNDMORE);
			char buf[200] = "uid";
			strcat(buf, test[2]);
			send_to_api(buf, 0);
		} else if (strcmp(test[1], "closed") == 0) {
			remove_cid(test[2], strlen(test[2]));
		} else {
			printf("wrong 2 frame:%s.\n", test[frames]);
		}

		for (int i = 0; i < frames; i++) {
			free(test[i]);
		}
	}

	zmq_close(pull);
}

