#include "comm_io_wraper.h"
#include "downstream.h"
#include "loger.h"
#include "message_concentrator.h"
#include "upstream.h"
#include "zmq_io_wraper.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

int message_fountain()
{
  while (1) {
    log("message_fountain.");
    upstream_msg();
  }
  return 0;
}

static void *_fountain_thread(void *usr)
{
  log("_fountain_thread.\n");
  assert(init_comm_io() == 0);
  assert(init_zmq_io() == 0);
  message_fountain();
  return NULL;
}

int concentrator_init(pthread_t *ntid)
{
  int err;
  err = pthread_create(ntid, NULL, _fountain_thread, NULL);
  if (err != 0) {
    printf("can't create thread:%s\n", strerror(err));
  }
  return err;
}

void concentrator_destroy()
{
  destroy_comm_io(); 
}
