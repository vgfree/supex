#pragma once

#include "comm_api.h"

int stream_i_wrap_init(void);

void stream_i_wrap_exit(void);

int stream_i_wrap_recv(struct comm_message *msg);

int stream_i_wrap_send(struct comm_message *msg);

