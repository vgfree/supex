#pragma once

#include "comm_api.h"

int message_comm_io_init(void);

void message_comm_io_exit(void);

int message_comm_io_recv(struct comm_message *msg);

int message_comm_io_send(struct comm_message *msg);
