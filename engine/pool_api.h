#pragma once

#include "cnt_pool.h"

bool pool_api_init(char *host, int port, int max, bool sync);

struct cnt_pool *pool_api_find(char *host, int port);

int pool_api_pull(struct cnt_pool *pool, void **cite, ...);

int pool_api_push(struct cnt_pool *pool, void **cite, ...);

int pool_api_free(struct cnt_pool *pool, void **cite, ...);

/*结合 pool_api_find 和 pool_api_pull*/
int pool_api_gain(struct cnt_pool **pool, char *host, int port, void **cite);

