#include <assert.h>
#include <stdio.h>
#include <zmq.h>

int main(int argc, char *argv[])
{
  void *ctx = zmq_ctx_new();
  void *server_simulator = zmq_socket(ctx, ZMQ_PULL);
  int rc = zmq_connect(server_simulator, "tcp://127.0.0.1:8102");
  assert(rc == 0);
  while (1) {
    int more;
    size_t more_size = sizeof(more);
    do {
      zmq_msg_t part;
      int rc = zmq_msg_init(&part);
      assert(rc == 0);
	  rc = zmq_recvmsg(server_simulator, &part, 0);
      printf("zmq_io_recv, rc:%d.\n", rc);
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
  zmq_ctx_destroy(ctx);
}
