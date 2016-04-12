//#include "communication.h"
#include "daemon.h"
#include "message_dispatch.h"
#include "loger.h"
#include "fd_manager.h"

#include <stdlib.h>

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

int main(int argc, char* argv[])
{ 
  g_imlog = CSLog_create(MODULE_NAME, WATCH_DELAY_TIME);
  log("init");
/*  if (unlikely(argc < 3)) {
    printf("usage: %s <ipaddr> <port>", argv[0]);
    return -1;
  }

  struct comm *commctx = NULL;
  commctx = comm_ctx_create(EPOLL_SIZE);
  if (unlikely(!commctx)) {
    return -1;
  }
  struct cbinfo  finishedcb = {};
  finishedcb.callback = io_notify_logic_thread;

  int retval = comm_socket(commctx, argv[1], argv[2], finishedcb, COMM_BIND);
  if (retval == -1) {
    error("can't bind socket.");
    return retval;
  }*/
  daemon_init(SERVER_FILE);
  log("message loop");
  while (1) {
    message_dispatch();
  }
  log("exit");
  daemon_exit(SERVER_FILE);
  CSLog_free(g_imlog);
}
