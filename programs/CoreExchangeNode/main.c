#include "config_reader.h"
#include "communication.h"
#include "daemon.h"
#include "message_dispatch.h"
#include "loger.h"
#include "fd_manager.h"

#include <signal.h>
#include <stdlib.h>

#define CONFIG "core_exchange_node.conf"
#define LISTEN_IP "ListenIP"
#define LISTEN_PORT "ListenPort"
#define SERVER_FILE "CoreExchangeNode.pid"
#define PACKAGE_SIZE "PackageSize"
#define MODULE_NAME "CoreExchangeNode"

struct CSLog* g_imlog = NULL;

void io_notify_logic_thread(struct comm_context* commctx,
                            struct portinfo *portinfo, void *usr)
{
  if (g_serv_info.commctx != commctx) {
    error("callback commctx not equal. g_serv_info.commctx:%p, commctx:%p",
          g_serv_info.commctx, commctx);
  }
  log("callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);
  switch (portinfo->stat) {
  case 0:
    break;
  case 1:   // connected.
	{
      struct fd_descriptor des;
      des.status = 1;
      array_fill_fd(portinfo->fd, &des);
	}
    break;
  case 4:   // closed.
    {
      struct fd_descriptor des;
      array_at_fd(portinfo->fd, &des);
      if (des.status != 1) {
        error("this fd:%d is not running.", portinfo->fd);
        return;
      }
      if (des.obj == CLIENT){
        // 打包路由结束包发送给其他服务器。message_from 为router_server.
        // TO DO:  更新到redis 服务器。
      }
      else if (des.obj == MESSAGE_GATEWAY) {
        list_remove(des.obj, portinfo->fd);
      }
      array_remove_fd(portinfo->fd);
      break;
    }
  default:
    break;
  }
}


static int init() {
  g_imlog = CSLog_create(MODULE_NAME, WATCH_DELAY_TIME);
  log("init");
  struct config_reader *config =
    init_config_reader(CONFIG);
  char *IP = get_config_name(config, LISTEN_IP);
  char *port = get_config_name(config, LISTEN_PORT);
  char *package_sz = get_config_name(config, PACKAGE_SIZE);
  log("ListenIp = %s, ListenPort = %s, pcakge size = %s", IP, port, package_sz);
  list_init();
  array_init();
  struct comm_context *commctx = NULL;
  commctx = comm_ctx_create(EPOLL_SIZE);
  if (unlikely(!commctx)) {
    return -1;
  }
  struct cbinfo  finishedcb = {};
  finishedcb.callback = io_notify_logic_thread;

  int retval = comm_socket(commctx, IP, port, &finishedcb, COMM_BIND);
  if (retval == -1) {
    error("can't bind socket.");
    return retval;
  }
  log("IP:%s", IP);
  get_ip(IP, g_serv_info.ip);
  g_serv_info.port = atoi(port);
  g_serv_info.package_size = atoi(package_sz);
  g_serv_info.commctx = commctx;
  log("port:%d", g_serv_info.port);
  destroy_config_reader(config);
  return 0;
}

int main(int argc, char* argv[])
{
  signal(SIGPIPE, SIG_IGN);
  if (daemon_init(SERVER_FILE) == 1) {
    printf("Server is running.");
    return -1;
  }
  if (init() == -1) {
    error("server init failed.");
    return -1;
  }
  log("");
  
  while (1) {
    log("message loop");
    message_dispatch();
  }
  array_destroy();
  list_destroy();
  daemon_exit(SERVER_FILE);
  CSLog_destroy(g_imlog);
  return 0;
}
