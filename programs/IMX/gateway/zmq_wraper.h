#pragma once

#include <stdlib.h>

void *zmq_push_init(void *ctx, char *host, int port);

void *zmq_pull_init(void *ctx, char *host, int port);
