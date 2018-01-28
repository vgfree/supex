#pragma once

#include "comm_api.h"

int manage_o_wrap_init(void);

void manage_o_wrap_exit(void);

int manage_o_wrap_recv(struct comm_message *msg);

int manage_o_wrap_send(struct comm_message *msg);

