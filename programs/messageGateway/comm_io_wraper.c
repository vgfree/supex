#include "comm_io_wraper.h"
#include "config_reader.h"
#include "loger.h"

#include "stdio.h"

#define CONFIG "messageGateway.conf"
#define NODE_IP "CoreExchangeNodeIP"
#define NODE_PORT "CoreExchangeNodePort"

static void core_exchange_node_cb(struct comm_context *commctx,
                                  struct portinfo *portinfo,
                                  void *usr)
{

}

static struct comm_context *g_commctx = NULL;
int init_comm_io()
{
  struct config_reader *config =
    init_config_reader(CONFIG);
  char *ip = get_config_name(config, NODE_IP);
  char *port = get_config_name(config, NODE_PORT);
  log("ip:%s. port:%s.", ip, port);
  g_commctx = comm_ctx_create(EPOLL_SIZE);
  if (!g_commctx) {
    return -1;
  }

  struct cbinfo callback_info = {};
  callback_info.callback = core_exchange_node_cb;

  char *ptr = port;
  int count = 1;
  while (*ptr != '\0' ) {
    if (*ptr == ';') {
      count++;
    }
  }
  char *ipptr = ip;
  char *portptr = port;
  for (int i = 0; i < count; i++) {
	char ipbuf[20];
	char portbuf[10];
	int j = 0;
	while (*ipptr != ';') {
      ipbuf[j] = *ipptr;
	  ipptr ++;
	  j++;
	}
	ipbuf[j] = '\0';
    j = 0;
	while (*portptr != ';') {
      portbuf[j] = *portptr;
      portptr ++;
      j++;
	}
    portbuf[j] = '\0';
    int retval = comm_socket(g_commctx, ipbuf, portbuf, &callback_info, COMM_CONNECT);
	if (retval == -1) {
      error("can't connect socket, ip:%s, port:%s.", ipbuf, portbuf);
      return -1;
	}
  }
  destroy_config_reader(config);
  return 0;
}

int recv_msg(struct comm_message *msg)
{
  assert(msg);
  return comm_recv(g_commctx, msg, false, -1);
}

int send_msg(struct comm_message *msg)
{
  assert(msg);
  return comm_send(g_commctx, msg, false, -1);
}
