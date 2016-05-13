#include "communication.h"
#include "comm_message_operator.h"
#include "core_exchange_node_test.h"
#include "loger.h"

int test_simulate_client()
{
  struct comm_context *ctx = comm_ctx_create(EPOLL_SIZE);
  struct cbinfo callback_info = {};
  callback_info.callback = NULL;
  int connectfd = comm_socket(ctx, "127.0.0.1", "8082", &callback_info, COMM_CONNECT);
  log("connectfd:%d", connectfd);
  if (connectfd == -1) {
    log("connect error.");
    return -1;
  }
  while (1) {
    struct comm_message msg = {};
    init_msg(&msg);
    set_msg_fd(&msg, connectfd);
    log("start send client.");
    set_msg_frame(0, &msg, 6, "client");
    comm_send(ctx, &msg, true, -1);
    destroy_msg(&msg);
    sleep(5);
  }
  return 0;
}
