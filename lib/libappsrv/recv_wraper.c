#include "recv_wraper.h"
#include "zmq.h"

#include <assert.h>

static void *s_gateway = NULL;
static void *s_login = NULL;

void init_recv(void *ctx)
{
  assert(!s_gateway && !s_login);
  s_login = zmq_socket(ctx, ZMQ_PULL);
  int rc = zmq_connect(s_login, "tcp://127.0.0.1:8101");
  assert(rc == 0);
  s_gateway = zmq_socket(ctx, ZMQ_PULL);
  rc = zmq_connect(s_gateway, "tcp://127.0.0.1:8092");
  assert(rc == 0);
}

void destroy_recv()
{
  assert(s_login && s_gateway);
  zmq_close(s_login);
  zmq_close(s_gateway);
}

int recv_all_msg(struct app_msg *msg, int *more, int flag)
{
  assert(msg && more);
  zmq_pollitem_t items[2];
  items[0].socket = s_login;
  items[0].events = ZMQ_POLLIN;
  items[1].socket = s_gateway;
  items[1].events = ZMQ_POLLIN;
  int rc = zmq_poll(items, 2, flag); // -1, block, 0,not block.
  assert(rc >= 0);
  if (rc == 2) {
    *more = 1;
  }
  else {
    *more = 0;
  }
  if (items[0].revents > 0) {
    return zmq_recviov(s_login, msg->vector, &msg->vector_size, ZMQ_DONTWAIT);
  }
  else if (items[1].revents > 0) {
    return zmq_recviov(s_gateway, msg->vector, &msg->vector_size, ZMQ_DONTWAIT);
  }
  return rc;
}
