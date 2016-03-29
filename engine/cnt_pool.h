#pragma once

#include <stdint.h>

#include "utils.h"

#define POOL_API_OK             0
#define POOL_API_ERR_NO_POOL    -1
#define POOL_API_ERR_IS_FULL    -2
#define POOL_API_ERR_OP_FAIL    -3

struct cnt_pool;
typedef int (*CNT_SOCK_OPT)(struct cnt_pool *pool, void **cite, va_list *ap);

struct cnt_pool
{
	int						max;
	int						use;	/*已使用数量*/
	char                    host[64];
	int                     port;
	CNT_SOCK_OPT            cso_open;
	CNT_SOCK_OPT            cso_exit;
	CNT_SOCK_OPT            cso_pull;
	CNT_SOCK_OPT            cso_push;
	struct free_queue_list  qlist;
	bool                    sync;	/*true 同步，false异步*/
};

int cnt_init(struct cnt_pool *pool, unsigned int max, bool sync, char *host, int port,
	CNT_SOCK_OPT cso_open, CNT_SOCK_OPT cso_exit,
	CNT_SOCK_OPT cso_pull, CNT_SOCK_OPT cso_push);

int cnt_pull(struct cnt_pool *pool, void **cite, void *data);

int cnt_push(struct cnt_pool *pool, void **cite, void *data);

int cnt_free(struct cnt_pool *pool, void **cite, void *data);

