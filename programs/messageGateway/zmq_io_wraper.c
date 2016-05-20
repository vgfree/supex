#include "config_reader.h"
#include "zmq_io_wraper.h"
#include "loger.h"

#include <assert.h>

#define CONFIG "messageGateway.conf"
#define PUSH_IP "PushServer"
#define PUSH_PORT "PushPort"
#define PULL_IP "PullServer"
#define PULL_PORT "PullPort"

static void *g_ctx = NULL;
static void *g_push = NULL;
static void *g_pull = NULL;
static void zmq_push_init(char *host, int port)
{
  assert(g_ctx);
  g_push = zmq_socket(g_ctx, ZMQ_PUSH);
  char addr[64] = {};
  sprintf(addr, "tcp://%s:%d", host, port);
  int rc = zmq_bind(g_push, addr);
  assert(rc == 0);
}

static int zmq_pull_init(char *host, int port)
{
  assert(g_ctx);
  g_pull = zmq_socket(g_ctx, ZMQ_PULL);
  char addr[64] = {};
  sprintf(addr, "tcp://%s:%d", host, port);
  log("addr:%s.", addr);
  int rc = zmq_bind(g_pull, addr);
  log("rc:%d", rc);
  return rc;
}

static void zmq_push_exit()
{
  zmq_close(g_push);
}

static void zmq_pull_exit()
{
  zmq_close(g_pull);
}

void zmq_exit()
{
  zmq_pull_exit();
  zmq_push_exit();
  zmq_ctx_destroy(g_ctx);
}

int zmq_io_send(zmq_msg_t *msg, int flags)
{
  return zmq_sendmsg(g_push, msg, flags);
}

int zmq_io_getsockopt(int option_name, void *option_value, size_t *option_len)
{
  return zmq_getsockopt(g_pull, option_name, option_value, option_len);
}
int zmq_io_recv(zmq_msg_t *msg, int flags)
{
  return zmq_recvmsg(g_pull, msg, flags);
}

int init_zmq_io()
{
  struct config_reader *config =
    init_config_reader(CONFIG);
  char *ip = get_config_name(config, PUSH_IP);
  char *port = get_config_name(config, PUSH_PORT);
  assert(!g_ctx);
  g_ctx = zmq_ctx_new();
  int intport = atoi(port);
  zmq_push_init(ip, intport);
  char *pullIp = get_config_name(config, PULL_IP);
  char *pullPort = get_config_name(config, PULL_PORT);
  log("pullIp:%s, clientPort:%s", pullIp, pullPort);
  zmq_pull_init(pullIp, atoi(pullPort));
  destroy_config_reader(config);
  return 0;
}
