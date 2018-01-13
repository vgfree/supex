#pragma once

#include "comm_api.h"

int login_comm_io_init(void);

void login_comm_io_exit(void);

int login_comm_io_recv(struct comm_message *msg);

int login_comm_io_send(struct comm_message *msg);
