#pragma once

#include "comm_api.h"

int manage_i_wrap_init(void);

void manage_i_wrap_exit(void);

int manage_i_wrap_recv(struct comm_message *msg);

int manage_i_wrap_send(struct comm_message *msg);

