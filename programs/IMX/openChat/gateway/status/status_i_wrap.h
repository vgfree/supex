#pragma once

#include "comm_api.h"

int status_i_wrap_init(void);

void status_i_wrap_exit(void);

int status_i_wrap_recv(struct comm_message *msg);

int status_i_wrap_send(struct comm_message *msg);

