#pragma once
#include "zmq.h"

int control_zmq_io_init(void);

void control_zmq_io_exit(void);

int control_zmq_io_recv(zmq_msg_t *msg, int flags);

enum zio_rw_type {
	ZIO_SEND_TYPE = 0,
	ZIO_RECV_TYPE,
};
int control_zmq_io_getsockopt(enum zio_rw_type rwopt, int option_name, void *option_value, size_t *option_len);
