//
//  connection_pool.h
//  supex
//
//  Created by 周凯 on 16/1/8.
//  Copyright © 2016年 zhoukai. All rights reserved.
//

#ifndef connection_pool_h
#define connection_pool_h

#include "../tcp_api/tcp_base.h"

__BEGIN_DECLS

struct ypool;
struct conn_ypool
{
	int                     magic;
	struct tcp_socket       *conn;
	struct ypool             *pool;
};

int conn_ypool_create(const char *host, const char *server, unsigned max, bool sync);

void conn_ypool_destroy(const char *host, const char *server);

int conn_ypool_pull(const char *host, const char *server, struct conn_ypool *pool);

void conn_ypool_push(struct conn_ypool *pool);

void conn_ypool_disconn(struct conn_ypool *pool);

__END_DECLS
#endif	/* connection_pool_h */

