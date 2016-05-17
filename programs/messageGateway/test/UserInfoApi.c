#include "simulate.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

void *api_thread(void *usr)
{
  void *server_simulator = zmq_socket(g_ctx, ZMQ_PUSH);
  int rc = zmq_bind(server_simulator, "tcp://127.0.0.1:8111");
  assert(rc == 0);
  while (1) {
    zmq_msg_t part1;
    int rc = zmq_msg_init_size(&part1, 7);
    assert(rc == 0);
    printf("api thread:%d, send msg 1 frame downstream.\n", thread_status[3]);
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
