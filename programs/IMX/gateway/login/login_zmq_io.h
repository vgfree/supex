#pragma once
#include "zmq.h"

int login_zmq_io_init(void);

void login_zmq_io_exit(void);

int login_zmq_io_send_to_app(zmq_msg_t *msg, int flags);

int login_zmq_io_send_to_api(zmq_msg_t *msg, int flags);

