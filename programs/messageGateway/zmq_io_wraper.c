#include "config_reader.h"
#include "zmq.h"
#include "zmq_io_wraper.h"

#include <assert.h>

#define CONFIG "messageGateway.conf"
#define CID_IP "CidServer"
#define CID_PORT "CidPort"
#define SERVER_SIZE 4

static void *g_ctx = NULL;
static void *g_server[SERVER_SIZE] = {NULL, NULL, NULL, NULL};
static void zmq_srv_init(char *host, int port, enum server srv)
{
  assert(g_ctx);
  g_server[srv] = zmq_socket(g_ctx, ZMQ_PUSH);
  char addr[64] = {};
  sprintf(addr, "tcp://%s:%d", host, port);
  int rc = zmq_bind(g_server[srv], addr);
  assert(rc == 0);
}

void zmq_srv_exit()
{
  for (int i = 0; i < SERVER_SIZE; i++) {
    if (g_server[i]) {
      zmq_close(g_server[i]);
	} 
  }
  zmq_ctx_destroy(g_ctx);
}

int zmq_send(enum server srv, zmq_msg_t *msg, int flags)
{
  return zmq_sendmsg(g_server[srv], msg, flags);
}

int init_zmq_io()
{
  struct config_reader *config =
    init_config_reader(CONFIG);
  char *ip = get_config_name(config, CID_IP);
  char *port = get_config_name(config, CID_PORT);
  assert(!g_ctx);
  g_ctx = zmq_ctx_new();
  zmq_srv_init(ip, port, CID_SERVER);
  return 0;
}
