#include "appsrv.h"
#include "recv_wraper.h"
#include "send_wraper.h"

#include <assert.h>
#include <string.h>

static void *g_ctx = NULL;
void create_io()
{
  assert(!g_ctx);
  g_ctx = zmq_ctx_new();
  init_send(g_ctx);
  init_recv(g_ctx);
}

void destroy_io()
{
  assert(g_ctx);
  destroy_send();
  destroy_recv();
  zmq_ctx_destroy(g_ctx);
}

int send_app_msg(struct app_msg *msg)
{
  assert(msg && msg->vector_size > 0);
  int rc = -1;
  printf("iov_len:%u\n", msg->vector[0].iov_len);
  if (msg->vector[0].iov_len == 7 &&
      memcmp("setting", msg->vector[0].iov_base, 7) == 0) {
     printf("send to _api.\n");
    rc = send_to_api(msg);
  }
  else {
    rc = send_to_gateway(msg);
  }
  return rc;
}

int recv_app_msg(struct app_msg *msg, int *more, int flag)
{
  assert(msg);
  return recv_all_msg(msg, more, flag);  
}
