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

enum command {
  FD_CONNECTED,
  FD_CLOSED
};

struct message_notify {
  enum command cmd; 
};

void io_notify_logic_thread(struct commctx* commctx, int fd,
							struct message_notify *msg_notify)
{
  switch (msg_notify->cmd) {
    // TO DO.
  case FD_CONNECTED:
	  break;
  case FD_CLOSED:
	  break;
  default:
	  break;
  }
}

struct CSLog* g_imlog = NULL;

static int init() {
  g_imlog = CSLog_create(MODULE_NAME, WATCH_DELAY_TIME);
  log("init");
  struct config_reader *config =
    init_config_reader(CONFIG);
  char *IP = get_config_name(config, LISTEN_IP);
  char *port = get_config_name(config, LISTEN_PORT);
  log("ListenIp = %s, ListenPort = %s.", IP, port);
  

  struct comm *commctx = NULL;
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
  return 0;
}

int main(int argc, char* argv[])
{
  signal(SIGPIPE, SIG_IGN);
  if (daemon_init(SERVER_FILE) == 1) {
	  printf("Server is running.");
  }
  if (init() == -1) {
    error("server init failed.");
	return -1;
  }
  
  while (1) {
	sleep(1);
    log("message loop");
    message_dispatch();
  }
  daemon_exit(SERVER_FILE);
  CSLog_free(g_imlog);
  return 0;
}
