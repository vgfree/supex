#include "comm_io_wraper.h"
#include "config_reader.h"
#include "loger.h"

#define CONFIG "settingServer.conf"
#define NODE_SERVER_IP "NodeServer"
#define NODE_SERVER_PORT "NodePort"

static void core_exchange_node_cb(struct comm_context *commctx,
                                  struct comm_tcp *portinfo,
                                  void *usr)
{
  log("callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);
  if (portinfo->stat == FD_INIT) {
    if (g_node_ptr->max_size > NODE_SIZE) {
      error("core exchange node:%d > max size:%d.", g_node_ptr->max_size, NODE_SIZE);
    }
    g_node_ptr->fd_array[g_node_ptr->max_size++] = portinfo->fd;
  }
  else if (portinfo->stat == FD_CLOSE) {
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
  g_commctx = comm_ctx_create(EPOLL_SIZE);
  if (!g_commctx) {
    return -1;
  }

  struct cbinfo callback_info = {};
  callback_info.callback = core_exchange_node_cb;

  char *nodeServer = get_config_name(config, NODE_SERVER_IP);
  char *nodePort = get_config_name(config, NODE_SERVER_PORT);
  log("nodeServer:%s, nodePort:%s.", nodeServer, nodePort);
  comm_socket(g_commctx, nodeServer, nodePort, &callback_info, COMM_BIND);
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
