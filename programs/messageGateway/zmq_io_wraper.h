#ifndef _ZMQ_IO_WRAPER_H_
#define _ZMQ_IO_WRAPER_H_

#include "zmq.h"

#include <stdlib.h>

enum server {
  APP_SERVER = 0
};

int init_zmq_io();
void zmq_exit();
int zmq_io_send(enum server srv, zmq_msg_t *msg, int flags);
int zmq_io_recv(zmq_msg_t *msg, int flags);
int zmq_io_getsockopt(int option_name, void *option_value, size_t *option_len);
#endif
