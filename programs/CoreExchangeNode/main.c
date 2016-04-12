//#include "communication.h"
#include "daemon.h"
#include "message_dispatch.h"
#include "loger.h"
#include "fd_manager.h"

#include <signal.h>
#include <stdlib.h>

#define SERVER_FILE "CoreExchangeNode.pid"
#define SERVER_PORT 8081

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

int main(int argc, char* argv[])
{
  signal(SIGPIPE, SIG_IGN);
  g_imlog = CSLog_create(MODULE_NAME, WATCH_DELAY_TIME);
  log("init");
  struct config_reader *config =
    init_config_reader("core_exchange_node.conf");
  log("config address:%p", config);
  log("init_config_reader over.");
  char *value = get_config_name(config, "ListenPort");
  log("ListenPort = %s.", value);
  

/*  struct comm *commctx = NULL;
  commctx = comm_ctx_create(EPOLL_SIZE);
  if (unlikely(!commctx)) {
    return -1;
  }
  struct cbinfo  finishedcb = {};
  finishedcb.callback = io_notify_logic_thread;

  int retval = comm_socket(commctx, argv[1], SERVER_PORT, finishedcb, COMM_BIND);
  if (retval == -1) {
    error("can't bind socket.");
    return retval;
  } */
  daemon_init(SERVER_FILE);
  log("message loop");
  while (1) {
    message_dispatch();
  }
  daemon_exit(SERVER_FILE);
  CSLog_free(g_imlog);
}
