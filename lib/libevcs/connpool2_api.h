#pragma once

#include <stdint.h>
#include "../base/utils.h"



bool pool_api_init(const char *host, int port, int max, bool sync);

void pool_api_destroy(const char *host, int port, ...);

struct pool2 *pool_api_find(const char *host, int port);

int pool_api_pull(struct pool2 *pool, void **cite, ...);

int pool_api_push(struct pool2 *pool, void **cite, ...);

int pool_api_free(struct pool2 *pool, void **cite, ...);

/*结合 pool_api_find 和 pool_api_pull*/
int pool_api_gain(struct pool2 **pool, char *host, int port, void **cite);

