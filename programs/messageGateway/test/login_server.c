#include "simulate.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <zmq.h>

void *login_thread(void *usr)
{
  void *sub = zmq_socket(g_ctx, ZMQ_SUB);
  int rc = zmq_bind(sub, "tcp://127.0.0.1:8101");
  assert(rc == 0);
  assert(zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "", 0) == 0);
  while (1) {
    int more;
    size_t more_size = sizeof(more);
    do {
      zmq_msg_t part;
      int rc = zmq_msg_init(&part);
      assert(rc == 0);
	  rc = zmq_recvmsg(sub, &part, 0);
      printf("login server:%d,zmq_io_recv, rc:%d.\n", thread_status[2], rc);
      char test[30] = {};
      memcpy(test, zmq_msg_data(&part), zmq_msg_size(&part));
      printf("recv data:%s.\n", test);
      assert(rc != -1);
      zmq_getsockopt(sub, ZMQ_RCVMORE, &more, &more_size);
      zmq_msg_close(&part);
      printf("more:%d.\n", more);
   } while (more);
  }
  zmq_close(sub);
}
