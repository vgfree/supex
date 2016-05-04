#ifndef _ZMQ_IO_WRAPER_H_
#define _ZMQ_IO_WRAPER_H_

#include <stdlib.h>

enum server {
  CID_SERVER = 3
};

int init_zmq_io();
int zmq_send(enum server srv, zmq_msg_t *msg, int flags);
#endif
