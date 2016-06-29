#include "comm_io_wraper.h"
#include "comm_message_operator.h"
#include "loger.h"
#include "upstream.h"
#include "zmq_io_wraper.h"

int upstream_msg()
{
	int i = 0;
	struct comm_message msg = {};

	init_msg(&msg);
	log("start to recv msg");
	recv_msg(&msg);
	log("get_max_msg_frame, :%d", get_max_msg_frame(&msg));

	for (i = 0; i < get_max_msg_frame(&msg); i++) {
		zmq_msg_t       msg_frame;
		int             fsz = 0;
		char            *frame = get_msg_frame(i, &msg, &fsz);
		int             rc = zmq_msg_init_size(&msg_frame, fsz);
		assert(rc == 0);
		memcpy(zmq_msg_data(&msg_frame), frame, fsz);

		if (i < get_max_msg_frame(&msg) - 1) {
			zmq_io_send(&msg_frame, ZMQ_SNDMORE);
		} else {
			zmq_io_send(&msg_frame, 0);
		}
	}

	for (i = 0; i < get_max_msg_frame(&msg); i++) {
		zmq_msg_t       msg_frame;
		int             fsz = 0;
		char            *frame = get_msg_frame(i, &msg, &fsz);
		int             rc = zmq_msg_init_size(&msg_frame, fsz);
		assert(rc == 0);
		memcpy(zmq_msg_data(&msg_frame), frame, fsz);

		if (i < get_max_msg_frame(&msg) - 1) {
			zmq_io_send_api(&msg_frame, ZMQ_SNDMORE);
		} else {
			zmq_io_send_api(&msg_frame, 0);
		}
	}

	destroy_msg(&msg);
	return 0;
}

