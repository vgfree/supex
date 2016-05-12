#include "config_reader.h"
#include "communication.h"
#include "daemon.h"
#include "fd_manager.h"
#include "gid_map.h"
#include "loger.h"
#include "message_dispatch.h"
#include "notify.h"
#include "uid_map.h"

#include <signal.h>
#include <stdlib.h>

#define CONFIG "core_exchange_node.conf"
#define LISTEN_MESSAGEGATEWAY_IP "ListenMessageGatewayIP"
#define LISTEN_MESSAGEGATEWAY_PORT "ListenMessageGatewayPort"
#define LISTEN_CLIENT_IP "ListenClientIP"
#define LISTEN_CLIENT_PORT "ListenClientPort"
#define SERVER_FILE "CoreExchangeNode.pid"
#define PACKAGE_SIZE "PackageSize"
#define MODULE_NAME "CoreExchangeNode"

struct CSLog* g_imlog = NULL;


static int init() {
  g_imlog = CSLog_create(MODULE_NAME, WATCH_DELAY_TIME);
  log("init");
  struct config_reader *config =
    init_config_reader(CONFIG);
  char *clientIP = get_config_name(config, LISTEN_CLIENT_IP);
  char *clientPort = get_config_name(config, LISTEN_CLIENT_PORT);
  char *MGsrvIP = get_config_name(config, LISTEN_MESSAGEGATEWAY_IP);
  char *MGsrvPort = get_config_name(config, LISTEN_MESSAGEGATEWAY_PORT);
//  char *package_sz = get_config_name(config, PACKAGE_SIZE);
  log("ListenClientIp = %s, ListenClientPort = %s", clientIP, clientPort);
  log("MGsrvIp = %s, MGsrvPort = %s", MGsrvIP, MGsrvPort);
  list_init();
  array_init();
  struct comm_context *commctx = NULL;
  commctx = comm_ctx_create(EPOLL_SIZE);
  if (unlikely(!commctx)) {
    return -1;
  }
  struct cbinfo  clientCB = {};
  clientCB.callback = client_event_notify;

  int retval = comm_socket(commctx, clientIP, clientPort, &clientCB, COMM_BIND);
  if (retval == -1) {
    error("can't bind client socket, ip:%s, port:%s.", clientIP, clientPort);
    return retval;
  }
  struct cbinfo MGCB = {};
  MGCB.callback = server_event_notify;
  retval = comm_socket(commctx, MGsrvIP, MGsrvPort, &MGCB, COMM_BIND);
  if (retval == -1) {
    error("can't bind MG socket, ip:%s, port:%s.", MGsrvIP, MGsrvPort);
  }
  get_ip(clientIP, g_serv_info.ip);
  g_serv_info.port = atoi(clientPort);
//  g_serv_info.package_size = atoi(package_sz);
  g_serv_info.commctx = commctx;
  log("port:%d", g_serv_info.port);
  destroy_config_reader(config);
  init_uid_map();
  init_gid_map();
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
  destroy_uid_map();
  destroy_gid_map();
  CSLog_destroy(g_imlog);
  daemon_exit(SERVER_FILE);
  return 0;
}
