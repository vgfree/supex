#pragma once

#include "comm_api.h"

int control_comm_io_init(void);

void control_comm_io_exit(void);

int control_comm_io_recv(struct comm_message *msg);

int control_comm_io_send(struct comm_message *msg);
