#pragma once

#include "comm_api.h"

int usrapi_o_wrap_init(void);

void usrapi_o_wrap_exit(void);

int usrapi_o_wrap_recv(struct comm_message *msg);

int usrapi_o_wrap_send(struct comm_message *msg);

