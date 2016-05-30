#include <assert.h>

#include "send_wraper.h"

static void *s_api = NULL;
static void *s_gateway = NULL;

void init_send(void *ctx)
{
  assert(!s_api && !s_gateway);
  s_api = zmq_socket(ctx, ZMQ_PUSH);
  int rc = zmq_connect(s_api, "tcp://127.0.0.1:8102");
  assert(rc == 0);

  s_gateway = zmq_socket(ctx, ZMQ_PUSH);
  rc = zmq_connect(s_gateway, "tcp://127.0.0.1:8090");
  assert(rc == 0);
}

void destroy_send()
{
  assert(s_api && s_gateway);
  zmq_close(s_api);
  zmq_close(s_gateway);
}

int send_to_api(struct app_msg *msg)
{
  assert(msg && s_api);
  return zmq_sendiov(s_api, msg->vector, msg->vector_size, ZMQ_DONTWAIT);
}

int send_to_gateway(struct app_msg *msg)
{
  assert(msg && s_gateway);
  return zmq_sendiov(s_gateway, msg->vector, msg->vector_size, ZMQ_DONTWAIT);
}

