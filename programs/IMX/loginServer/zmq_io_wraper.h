#ifndef _ZMQ_IO_WRAPER_H_
#define _ZMQ_IO_WRAPER_H_

#include "zmq.h"

#include <stdlib.h>

int init_zmq_io(void);

void exit_zmq_io(void);

int zmq_io_send_app(zmq_msg_t *msg, int flags);

int zmq_io_send_api(zmq_msg_t *msg, int flags);

#endif

