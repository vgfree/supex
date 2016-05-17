#include "config_reader.h"
#include "zmq_io_wraper.h"
#include "loger.h"

#include <assert.h>

#define CONFIG "loginServer.conf"
#define APP_IP "AppIP"
#define APP_PORT "AppPort"

static void *g_ctx = NULL;
static void *g_server = NULL;
static void zmq_srv_init(char *host, int port)
{
  assert(g_ctx);
  g_server = zmq_socket(g_ctx, ZMQ_PUB);
  char addr[64] = {};
  sprintf(addr, "tcp://%s:%d", host, port);
  log("addr:%s.", addr);
  int rc = zmq_bind(g_server, addr);
  assert(rc == 0);
}

static void zmq_srv_exit()
{
  zmq_close(g_server);
}

void zmq_exit()
{
  zmq_srv_exit();
  zmq_ctx_destroy(g_ctx);
}

int zmq_io_send(zmq_msg_t *msg, int flags)
{
  return zmq_sendmsg(g_server, msg, flags);
}

int init_zmq_io()
{
  struct config_reader *config =
    init_config_reader(CONFIG);
  char *ip = get_config_name(config, APP_IP);
  char *port = get_config_name(config, APP_PORT);
  assert(!g_ctx);
  g_ctx = zmq_ctx_new();
  int intport = atoi(port);
  zmq_srv_init(ip, intport);
  destroy_config_reader(config);
  return 0;
}
