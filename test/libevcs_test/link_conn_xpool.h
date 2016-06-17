/* @author: qianye@mirrtalk.com */

#ifndef LINK_CONN_XPOOL_H
#define LINK_CONN_XPOOL_H

#include "pool_api/conn_xpool_api.h"
#include "link_hashmap.h"
#include "json.h"

/*struct xpool            *cpool = conn_xpool_find("192.168.1.12", 9001);*/

typedef struct {
  link_hashmap_t *hashmap;
  struct json_object *cfg;
} link_conn_xpool_t;

link_conn_xpool_t *link_create_conn_xpool(const char *filename);
void link_destory_conn_xpool(link_conn_xpool_t *pool);

struct xpool *link_get_xpool_by_name(link_conn_xpool_t *pool, const char *name);

struct xpool *link_get_xpool_by_hash(link_conn_xpool_t *pool, const char *name, const char* hash);

#endif	/* LINK_CONN_XPOOL_H */
