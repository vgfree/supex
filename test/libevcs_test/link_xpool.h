/* @author: qianye@mirrtalk.com */

#ifndef LINK_CONN_XPOOL_H
#define LINK_CONN_XPOOL_H

#include "pool_api/conn_xpool_api.h"
#include "async_tasks/async_obj.h"

int link_xpool_init(const char *filename);
void link_xpool_destory(void);

/*struct xpool *link_get_redis_xpool_by_name(const char *name);*/
struct xpool *link_get_html_xpool_by_name(const char *name);

/*struct xpool *link_get_xpool_by_hash(const char *name, const char* hash);*/
struct xpool *link_get_xpool_by_conhash(const char *name, const char* hash);

#endif	/* LINK_CONN_XPOOL_H */
