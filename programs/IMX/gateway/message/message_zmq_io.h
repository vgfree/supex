#pragma once

int message_zmq_io_init(void);

void message_zmq_io_exit(void);

int message_zmq_io_send(zmq_msg_t *msg, int flags);

int message_zmq_io_recv(zmq_msg_t *msg, int flags);

enum zio_rw_type {
	ZIO_SEND_TYPE = 0,
	ZIO_RECV_TYPE,
};
int message_zmq_io_getsockopt(enum zio_rw_type rwopt, int option_name, void *option_value, size_t *option_len);
