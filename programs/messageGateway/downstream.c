#include "comm_io_wraper.h"
#include "comm_message_operator.h"
#include "downstream.h"
#include "loger.h"
#include "zmq_io_wraper.h"

int downstream_msg(struct comm_message *msg)
{
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
  int more;
  size_t more_size = sizeof(more);
  int i = 0;
  do {
    zmq_msg_t part;
    int rc = zmq_msg_init(&part);
    assert(rc == 0);
	rc = zmq_io_recv(&part, 0);
    assert(rc != -1);
    set_msg_frame(i, msg, zmq_msg_size(&part), zmq_msg_data(&part));
    zmq_io_getsockopt(ZMQ_RCVMORE, &more, &more_size);
    assert(rc == 0);
    zmq_msg_close(&part);
    i++;
  } while (more);
  return 0;
}
