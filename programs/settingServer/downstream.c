#include "comm_io_wraper.h"
#include "comm_message_operator.h"
#include "downstream.h"
#include "loger.h"
#include "zmq_io_wraper.h"

int downstream_msg(struct comm_message *msg)
{
	log("core exchange node max size:%d.", g_node_ptr->max_size);

	/*log("get_max_msg_frame:%d", get_max_msg_frame(msg));
	 *   int j = 0;
	 *   for (j = 0; j < msg->package.frames; j++) {
	 *   log("frame[%d]:frame_size:%d, frame_offset:%d\n", j, msg->package.frame_size[j], msg->package.frame_offset[j]);
	 *   }
	 *   log("frames_of_package：%d packages:%d dsize:%d\n", msg->package.frames_of_package[0], msg->package.packages, msg->package.dsize);*/
	for (int i = 0; i < g_node_ptr->max_size; i++) {
		msg->fd = g_node_ptr->fd_array[i];

		if (send_msg(msg) == -1) {
			error("wrong msg, msg fd:%d.", msg->fd);
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
		log("zmq_io_recv, rc:%d.", rc);

		/*char test[3000] = {};
		 *   memcpy(test, zmq_msg_data(&part), zmq_msg_size(&part));
		 *   log("recv data:%s.", test);*/
		assert(rc != -1);
		set_msg_frame(i, msg, zmq_msg_size(&part), zmq_msg_data(&part));
		zmq_io_getsockopt(ZMQ_RCVMORE, &more, &more_size);
		zmq_msg_close(&part);
		log("more:%d.", more);
		i++;
	} while (more);
	return 0;
}

