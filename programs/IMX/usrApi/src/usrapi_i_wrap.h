#pragma once

#include "comm_api.h"

int usrapi_i_wrap_init(void);

void usrapi_i_wrap_exit(void);

int usrapi_i_wrap_recv(struct comm_message *msg);

int usrapi_i_wrap_send(struct comm_message *msg);

