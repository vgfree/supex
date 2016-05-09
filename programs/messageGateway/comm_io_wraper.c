#include "comm_io_wraper.h"
#include "config_reader.h"
#include "loger.h"
#include "validate_logon.h"

#define CONFIG "messageGateway.conf"
#define NODE_IP "CoreExchangeNodeIP"
#define NODE_PORT "CoreExchangeNodePort"

static void core_exchange_node_cb(struct comm_context *commctx,
                                  struct portinfo *portinfo,
                                  void *usr)
{
  log("callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);
  if (portinfo->stat == FD_INIT) {
    if (g_node_ptr->max_size > NODE_SIZE) {
      error("core exchange node:%d > max size:%d.", g_node_ptr->max_size, NODE_SIZE);
    }
    g_node_ptr->fd_array[g_node_ptr->max_size++] = portinfo->fd;
  }
  else if (portinfo->stat == FD_CLOSE){
    int i = 0;
    for (; i < g_node_ptr->max_size; i++) {
      if (g_node_ptr->fd_array[i] == portinfo->fd) {
        break;
      }
    }
    for (; i < g_node_ptr->max_size - 1; i++) {
      g_node_ptr->fd_array[i] = g_node_ptr->fd_array[i + 1];
    }
    if (g_node_ptr->max_size == i) {
      error("not g_node fd_array no include this fd:%d.", portinfo->fd);
    }
    else {
      g_node_ptr->max_size--;
    }
  }
}

static struct comm_context *g_commctx = NULL;
int init_comm_io()
{
  g_node_ptr = (struct core_exchange_node *)malloc(sizeof(struct core_exchange_node));
  g_node_ptr->max_size = 0;
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
    ptr++;
  }
  log("node server number:%d", count);
  char *ipptr = ip;
  char *portptr = port;
  for (int i = 0; i < count; i++) {
    char ipbuf[20];
    char portbuf[10];
    int j = 0;
    while (*ipptr != ';') {
      if (*ipptr == '\0') {
        break;
      }
      ipbuf[j] = *ipptr;
      ipptr++;
      j++;
    }
    ipptr++;
    ipbuf[j] = '\0';
    j = 0;
    while (*portptr != ';') {
      if (*portptr == '\0') {
        break;
      }
      portbuf[j] = *portptr;
      portptr++;
      j++;
    }
    portptr++;
    portbuf[j] = '\0';
    log("comm_socket before:ip:%s, port:%s.", ipbuf, portbuf);
    int connectfd = comm_socket(g_commctx, ipbuf, portbuf, &callback_info, COMM_CONNECT);
    log("connectfd:%d", connectfd);
    if (connectfd == -1) {
      error("can't connect socket, ip:%s, port:%s.", ipbuf, portbuf);
      continue;
    }
    send_validate_logon_package(connectfd);
  }
  destroy_config_reader(config);
  return 0;
}

void destroy_comm_io()
{
  free(g_node_ptr);
}

int recv_msg(struct comm_message *msg)
{
  assert(msg);
  return comm_recv(g_commctx, msg, true, -1);
}

int send_msg(struct comm_message *msg)
{
  assert(msg);
  return comm_send(g_commctx, msg, false, -1);
}
