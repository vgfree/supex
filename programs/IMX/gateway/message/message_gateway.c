#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "libmini.h"
#include "comm_api.h"
#include "message_comm_io.h"
#include "message_zmq_io.h"
#include "message_sockfd_manage.h"

static void do_msg_for_each_fcb(int sockfd, int idx, void *usr)
{
	struct comm_message *msg = usr;
	commmsg_sets(msg, sockfd, 0, PUSH_METHOD);
	if (message_comm_io_send(msg) == -1) {
		x_printf(E, "wrong msg, msg fd:%d.", msg->fd);
	}
}

static int downstream_msg(struct comm_message *msg)
{
	int cnt = message_sockfd_manage_travel(do_msg_for_each_fcb, NULL, msg);
	x_printf(D, "core exchange node travel count:%d.", cnt);
	return 0;
}

static int pull_msg(struct comm_message *msg)
{
	assert(msg);
	int     more = 0;
	size_t  more_size = sizeof(more);
	int     i = 0;
	do {
		zmq_msg_t       part;
		int             rc = zmq_msg_init(&part);
		assert(rc == 0);
		rc = message_zmq_io_recv(&part, 0);
		assert(rc != -1);

		commmsg_frame_set(msg, i, zmq_msg_size(&part), zmq_msg_data(&part));
		msg->package.frames_of_package[0] = msg->package.frames;
		msg->package.packages = 1;

		zmq_msg_close(&part);

		message_zmq_io_getsockopt(ZIO_RECV_TYPE, ZMQ_RCVMORE, &more, &more_size);
		x_printf(D, "more:%d more_size %ld.", more, more_size);
		i++;
	} while (more);

	x_printf(D, "commmsg_frame_count:%d", commmsg_frame_count(msg));
#if 0
	int j = 0;
	for (j = 0; j < msg->package.frames; j++) {
		x_printf(D, "frame[%d]:frame_size:%d, frame_offset:%d\n", j, msg->package.frame_size[j], msg->package.frame_offset[j]);
	}
	x_printf(D, "frames_of_package：%d packages:%d dsize:%d\n", msg->package.frames_of_package[0], msg->package.packages, msg->package.dsize);
#endif
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

static int upstream_msg(struct comm_message *msg)
{
	int i = 0;
	for (i = 0; i < commmsg_frame_count(msg); i++) {
		zmq_msg_t       msg_frame;
		int             fsz = 0;
		char            *frame = commmsg_frame_get(msg, i, &fsz);
		int             rc = zmq_msg_init_size(&msg_frame, fsz);
		assert(rc == 0);
		memcpy(zmq_msg_data(&msg_frame), frame, fsz);

		if (i < commmsg_frame_count(msg) - 1) {
			message_zmq_io_send(&msg_frame, ZMQ_SNDMORE);
		} else {
			message_zmq_io_send(&msg_frame, 0);
		}
	}
	return 0;
}

static void *_push_thread(void *usr)
{
	while (1) {
		struct comm_message msg = {};

		commmsg_make(&msg, DEFAULT_MSG_SIZE);
		message_comm_io_recv(&msg);
		x_printf(D, "commmsg_frame_count, :%d", commmsg_frame_count(&msg));

		upstream_msg(&msg);

		commmsg_free(&msg);
	}

	return NULL;
}


static pthread_t tid1;
static pthread_t tid2;
void message_gateway_work(void)
{
	assert(message_comm_io_init() == 0);
	assert(message_zmq_io_init() == 0);

	/*work pull*/
	int err = pthread_create(&tid1, NULL, _pull_thread, NULL);
	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}

	/*work push*/
	err = pthread_create(&tid2, NULL, _push_thread, NULL);
	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}
	x_printf(I, "message gateway work!\n");
	return;
}

void message_gateway_wait(void)
{
	/*over*/
	void *status = NULL;
	pthread_join(tid1, status);
	pthread_join(tid2, status);
}


void message_gateway_stop(void)
{
	x_printf(W, "message gateway stop!\n");
	message_comm_io_exit();
	message_zmq_io_exit();
}

