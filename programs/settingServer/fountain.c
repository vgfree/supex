#include "comm_io_wraper.h"
#include "downstream.h"
#include "loger.h"
#include "fountain.h"
#include "zmq_io_wraper.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void message_fountain()
{
  while (1) {
    struct comm_message msg = {};
    init_msg(&msg);
    pull_msg(&msg);
    downstream_msg(&msg);
    destroy_msg(&msg);
  }
}

int fountain_init()
{
  assert(init_comm_io() == 0);
  assert(init_zmq_io() == 0);
  return 0;
}

void fountain_destroy()
{
  destroy_comm_io(); 
  zmq_exit();
}
