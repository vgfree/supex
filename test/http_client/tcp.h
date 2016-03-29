#pragma once

#include "wrap.h"
#include <netinet/in.h>

int tcp_connect(char *ip, int port);

ssize_t tcp_send(int fd, void *data, size_t len);

ssize_t tcp_receive(int fd, void *buf, size_t bytes);

