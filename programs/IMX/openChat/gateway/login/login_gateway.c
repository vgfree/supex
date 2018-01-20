#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include "libmini.h"
#include "comm_api.h"
#include "comm_print.h"
#include "login_comm_io.h"
#include "login_zmq_io.h"
#include "login_sockfd_manage.h"

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
			login_zmq_io_send_to_app(&msg_frame, ZMQ_SNDMORE);
		} else {
			login_zmq_io_send_to_app(&msg_frame, 0);
		}
	}
#if 1
	for (i = 0; i < commmsg_frame_count(msg); i++) {
		zmq_msg_t       msg_frame;
		int             fsz = 0;
		char            *frame = commmsg_frame_get(msg, i, &fsz);
		int             rc = zmq_msg_init_size(&msg_frame, fsz);
		assert(rc == 0);
		memcpy(zmq_msg_data(&msg_frame), frame, fsz);

		if (i < commmsg_frame_count(msg) - 1) {
			login_zmq_io_send_to_api(&msg_frame, ZMQ_SNDMORE);
		} else {
			login_zmq_io_send_to_api(&msg_frame, 0);
		}
	}
#endif
	return 0;
}

static void *_push_thread(void *usr)
{
	while (1) {
		struct comm_message msg = {};

		commmsg_make(&msg, DEFAULT_MSG_SIZE);
		login_comm_io_recv(&msg);
#define debug 1
#ifdef debug
		x_printf(D, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
		commmsg_print(&msg);
		x_printf(D, ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
#endif

		upstream_msg(&msg);

		commmsg_free(&msg);
	}

	return NULL;
}


static pthread_t tid1;
void login_gateway_work(void)
{
	assert(login_comm_io_init() == 0);
	assert(login_zmq_io_init() == 0);

	/*work push*/
	int err = pthread_create(&tid1, NULL, _push_thread, NULL);
	if (err != 0) {
		x_printf(E, "can't create pull thread:%s.", strerror(err));
	}
	x_printf(I, "login gateway work!\n");
	return;
}

void login_gateway_wait(void)
{
	/*over*/
	void *status = NULL;
	pthread_join(tid1, status);
}


void login_gateway_stop(void)
{
	x_printf(W, "login gateway stop!\n");
	login_comm_io_exit();
	login_zmq_io_exit();
}

