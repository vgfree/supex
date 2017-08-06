#include "comm_io_wraper.h"
#include "downstream.h"
#include "zmq_io_wraper.h"
#include "comm_message_operator.h"
#include "libmini.h"

int downstream_msg(struct comm_message *msg)
{
	x_printf(D, "core exchange node max size:%d.", g_node_ptr->max_size);
	x_printf(D, "commmsg_frame_count:%d", commmsg_frame_count(msg));

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
		commmsg_frame_set(msg, i, zmq_msg_size(&part), zmq_msg_data(&part));
		msg.package.frames_of_package[0] = msg.package.frames;
		msg.package.packages = 1;
		zmq_io_getsockopt(ZMQ_RCVMORE, &more, &more_size);
		zmq_msg_close(&part);
		x_printf(D, "more:%d.", more);
		i++;
	} while (more);
	return 0;
}

