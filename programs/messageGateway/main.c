#include "daemon.h"
#include "http_dispatch.h"
#include "loger.h"
#include "message_concentrator.h"

#include <pthread.h>
#include <signal.h>

#define SERVER_FILE "messageGateway.pid"
#define MODULE_NAME "messageGateway"

struct CSLog *g_imlog = NULL;
int main(int argc, char *argv[])
{
  signal(SIGPIPE, SIG_IGN);
  if (daemon_init(SERVER_FILE) == 1) {
    printf("server is running");
    return -1;
  }
  g_imlog = CSLog_create(MODULE_NAME, WATCH_DELAY_TIME);
  pthread_t tid;
  if (concentrator_init(&tid) != 0) {
    printf("concentrator not init.");
    return -1;
  }
  http_run(argc, argv);
  void *status;
  pthread_join(tid, &status);
  concentrator_destroy();
  daemon_exit(SERVER_FILE);
  CSLog_destroy(g_imlog);
  return 0;
}
