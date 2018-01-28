#pragma once

#include "comm_api.h"

int appsrv_o_wrap_init(void);

void appsrv_o_wrap_exit(void);

int appsrv_o_wrap_recv(struct comm_message *msg);

int appsrv_o_wrap_send(struct comm_message *msg);

