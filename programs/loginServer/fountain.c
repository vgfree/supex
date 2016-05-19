#include "comm_io_wraper.h"
#include "loger.h"
#include "fountain.h"
#include "upstream.h"
#include "zmq_io_wraper.h"

#include <stdlib.h>
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
