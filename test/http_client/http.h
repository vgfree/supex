#pragma once
#include <string.h>
#include "tcp.h"

enum method
{
	GET = 0,
	POST,
};

void *http_request(short method, char *url, ssize_t *back_size, void *data, size_t len);

