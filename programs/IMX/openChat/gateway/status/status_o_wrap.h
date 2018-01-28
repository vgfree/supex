#pragma once

#include "comm_api.h"

int status_o_wrap_init(void);

void status_o_wrap_exit(void);

int status_o_wrap_recv(struct comm_message *msg);

int status_o_wrap_send(struct comm_message *msg);

int status_o_get_api_sfd(void);

