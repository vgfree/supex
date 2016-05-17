#include "simulate.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

void *pull_thread(void *usr)
{
  void *server_simulator = zmq_socket(g_ctx, ZMQ_PULL);
  int rc = zmq_connect(server_simulator, "tcp://127.0.0.1:8092");
  assert(rc == 0);
  while (1) {
    int more;
    size_t more_size = sizeof(more);
    do {
      zmq_msg_t part;
      int rc = zmq_msg_init(&part);
      assert(rc == 0);
	  rc = zmq_recvmsg(server_simulator, &part, 0);
      printf("pull server:%d, zmq_io_recv, rc:%d.\n", thread_status[1], rc);
      char test[30] = {};
      memcpy(test, zmq_msg_data(&part), zmq_msg_size(&part));
      printf("recv data:%s.\n", test);
      assert(rc != -1);
      zmq_getsockopt(server_simulator, ZMQ_RCVMORE, &more, &more_size);
      zmq_msg_close(&part);
      printf("more:%d.\n", more);
   } while (more);
  }
  zmq_close(server_simulator);

}

void *push_thread(void *usr)
{
  void *server_simulator = zmq_socket(g_ctx, ZMQ_PUSH);
  int rc = zmq_bind(server_simulator, "tcp://127.0.0.1:8090");
  assert(rc == 0);
  while (1) {
    zmq_msg_t part1;
    int rc = zmq_msg_init_size(&part1, 7);
    assert(rc == 0);
    printf("push server:%d. send msg 1 frame downstream.\n", thread_status[0]);
    memcpy(zmq_msg_data(&part1), "setting", 7);
    zmq_sendmsg(server_simulator, &part1, ZMQ_SNDMORE);
    zmq_msg_t part2;
    rc = zmq_msg_init_size(&part2, 6);
    assert(rc == 0);
    printf("send msg 2 frame cid.\n");
    memcpy(zmq_msg_data(&part2), "status", 6);
    zmq_sendmsg(server_simulator, &part2, ZMQ_SNDMORE);
    zmq_msg_t part3;
    char cid[6] = {0x7f, 0x00, 0x00, 0x01, 0x00, 0x09};
    rc = zmq_msg_init_size(&part3, 6);
    printf("send msg 3 frame content.\n");
    memcpy(zmq_msg_data(&part3), cid, 6);
    zmq_sendmsg(server_simulator, &part3, ZMQ_SNDMORE);

    zmq_msg_t part4;
    zmq_msg_init_size(&part4, 6);
    memcpy(zmq_msg_data(&part4), "closed", 6);
    zmq_sendmsg(server_simulator, &part4, 0);

	sleep(5);
  }
  zmq_close(server_simulator);
}
