#include "communication.h"
#include "downstream.h"
#include "loger.h"
#include "zmq_pull_thread.h"

#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#define MAX_MSG_SIZE 10240

static void *_pull_thread(void *usr)
{
  while (1) {
    struct comm_message msg;
    msg.content = (char *)malloc(MAX_MSG_SIZE);
    pull_msg(&msg);
    downstream_msg(&msg);
    free(msg.content);
  }
  return NULL;
}

int pull_thread(pthread_t *ntid)
{
  
  int err = pthread_create(ntid, NULL, _pull_thread, NULL);
  if (err != 0) {
    error("can't create pull thread:%s.", strerror(err));
  }
  return err;
}
