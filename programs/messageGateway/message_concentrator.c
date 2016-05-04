#include "comm_io_wraper.h"
#include "downstream.h"
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
    upstream_msg();
    downstream_msg();
  }
  return 0;
}

static void *_fountain_thread(void *usr)
{
  assert(init_comm_io() == 0);
  assert(init_zmq_io() == 0);
  message_fountain();
}

int concentrator_init(pthread_t &ntid)
{
  int err;
  err = pthread_create(&ntid, NULL, _fountain_thread, NULL);
  if (err != 0) {
    printf("can't create thread:%s\n", strerror(err));
  }
  return err;
}
