#pragma once

#include "comm_api.h"

int stream_o_wrap_init(void);

void stream_o_wrap_exit(void);

int stream_o_wrap_recv(struct comm_message *msg);

int stream_o_wrap_send(struct comm_message *msg);

