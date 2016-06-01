#pragma once

#include <stdint.h>
#include "../base/utils.h"

bool conn_xpool_init(const char *host, int port, int max, bool sync);

void conn_xpool_destroy(const char *host, int port, ...);

struct xpool    *conn_xpool_find(const char *host, int port);

int conn_xpool_pull(struct xpool *pool, void **cite, ...);

int conn_xpool_push(struct xpool *pool, void **cite, ...);

int conn_xpool_free(struct xpool *pool, void **cite, ...);

/*结合 conn_xpool_find 和 conn_xpool_pull*/
int conn_xpool_gain(struct xpool **pool, char *host, int port, void **cite);

