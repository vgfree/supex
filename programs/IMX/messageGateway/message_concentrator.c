#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "comm_api.h"
#include "comm_io_wraper.h"
#include "zmq_io_wraper.h"
#include "comm_message_operator.h"
#include "libmini.h"

int downstream_msg(struct comm_message *msg)
{
	x_printf(D, "core exchange node max size:%d.", g_node_ptr->max_size);
	x_printf(D, "get_max_msg_frame:%d", get_max_msg_frame(msg));

	/*  int j = 0;
	 *   for (j = 0; j < msg->package.frames; j++) {
	 *          x_printf(D, "frame[%d]:frame_size:%d, frame_offset:%d\n", j, msg->package.frame_size[j], msg->package.frame_offset[j]);
	 *   }
	 *   x_printf(D, "frames_of_package：%d packages:%d dsize:%d\n", msg->package.frames_of_package[0], msg->package.packages, msg->package.dsize);*/
	for (int i = 0; i < g_node_ptr->max_size; i++) {
		commmsg_sets(msg, g_node_ptr->fd_array[i], 0, PUSH_METHOD);

		if (send_msg(msg) == -1) {
			x_printf(E, "wrong msg, msg fd:%d.", msg->fd);
			int j = i;
			for (; j < g_node_ptr->max_size - 1; j++) {
				g_node_ptr->fd_array[j] = g_node_ptr->fd_array[j + 1];
			}
			i--;
			g_node_ptr->max_size--;
		}
	}

	return 0;
}

int pull_msg(struct comm_message *msg)
{
	assert(msg);
	int     more;
	size_t  more_size = sizeof(more);
	int     i = 0;
	do {
		zmq_msg_t       part;
		int             rc = zmq_msg_init(&part);
		assert(rc == 0);
		rc = zmq_io_recv(&part, 0);
		//    x_printf(D, "zmq_io_recv, rc:%d.", rc);
		assert(rc != -1);

		x_printf(D, "more_size:%ld.", more_size);
		set_msg_frame(i, msg, zmq_msg_size(&part), zmq_msg_data(&part));
		zmq_io_getsockopt(ZMQ_RCVMORE, &more, &more_size);
		zmq_msg_close(&part);
		x_printf(D, "more:%d.", more);
		i++;
	} while (more);
	return 0;
}



static void *_pull_thread(void *usr)
{
	while (1) {
		struct comm_message msg = {};
		commmsg_make(&msg, DEFAULT_MSG_SIZE);
		pull_msg(&msg);
		downstream_msg(&msg);
		commmsg_free(&msg);
	}

	return NULL;
}

int upstream_msg(void)
{
	int i = 0;
	struct comm_message msg = {};

	commmsg_make(&msg, DEFAULT_MSG_SIZE);
	recv_msg(&msg);
	x_printf(D, "get_max_msg_frame, :%d", get_max_msg_frame(&msg));

	for (i = 0; i < get_max_msg_frame(&msg); i++) {
		zmq_msg_t       msg_frame;
		int             fsz = 0;
		char            *frame = commmsg_frame_get(&msg, i, &fsz);
		int             rc = zmq_msg_init_size(&msg_frame, fsz);
		assert(rc == 0);
		memcpy(zmq_msg_data(&msg_frame), frame, fsz);

		if (i < get_max_msg_frame(&msg) - 1) {
			zmq_io_send(&msg_frame, ZMQ_SNDMORE);
		} else {
			zmq_io_send(&msg_frame, 0);
		}
	}

	commmsg_free(&msg);
	return 0;
}

static void *_push_thread(void *usr)
{
	while (1) {
		upstream_msg();
	}

	return NULL;
}


void concentrator_work(void)
{
	g_node_ptr = (struct core_exchange_node *)malloc(sizeof(struct core_exchange_node));
	g_node_ptr->max_size = 0;

	assert(init_comm_io() == 0);
	assert(init_zmq_io() == 0);

	/*work pull*/
	pthread_t tid1;
	int err = pthread_create(&tid1, NULL, _pull_thread, NULL);
	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}

	/*work push*/
	pthread_t tid2;
	err = pthread_create(&tid2, NULL, _push_thread, NULL);
	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}

	/*over*/
	void *status = NULL;
	pthread_join(tid1, status);
	pthread_join(tid2, status);
	return;
}


void concentrator_destroy()
{
	exit_comm_io();
	exit_zmq_io();
}

