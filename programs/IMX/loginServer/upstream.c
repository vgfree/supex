#include "comm_io_wraper.h"
#include "comm_message_operator.h"
#include "libmini.h"
#include "upstream.h"
#include "zmq_io_wraper.h"

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
			zmq_io_send_app(&msg_frame, ZMQ_SNDMORE);
		} else {
			zmq_io_send_app(&msg_frame, 0);
		}
	}

	for (i = 0; i < get_max_msg_frame(&msg); i++) {
		zmq_msg_t       msg_frame;
		int             fsz = 0;
		char            *frame = commmsg_frame_get(&msg, i, &fsz);
		int             rc = zmq_msg_init_size(&msg_frame, fsz);
		assert(rc == 0);
		memcpy(zmq_msg_data(&msg_frame), frame, fsz);

		if (i < get_max_msg_frame(&msg) - 1) {
			zmq_io_send_api(&msg_frame, ZMQ_SNDMORE);
		} else {
			zmq_io_send_api(&msg_frame, 0);
		}
	}

	commmsg_free(&msg);
	return 0;
}

