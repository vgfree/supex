/*conn_xpool_init("192.168.1.12", 9001, 10, true);*/

#include "link_cfg_parser.h"

link_hashmap_t *xpool_hashmap;

link_conn_xpool_t *link_create_conn_xpool(const char *filename) {
  struct json_object *cfg = json_object_from_file(filename);
  if (cfg == NULL) {
	  x_printf(E, "Invalid config file :%s", filename);
    return NULL;
  }

  link_hashmap_t *hashmap = link_hashmap_create_str_pointer(LINK_128_SHIFT);
  if (hashmap == NULL) {
	  x_printf(E, "No memory for hashmap");
    return NULL;
  }

  link_conn_xpool_t *pool = malloc(sizeof(*pool));
  if (pool == NULL) {
	  x_printf(E, "No memory for pool");
    return NULL;
  }

  pool->cfg = cfg;
  pool->hashmap = hashmap;
  return pool;
}

void link_destory_conn_xpool(link_conn_xpool_t *pool) {
  if (pool) {
    if (pool->cfg) {
      json_object_put(pool->cfg);
    }

    if (pool->hashmap) {
      link_hashmap_destory(pool->hashmap);
    }

    free(pool);
  }
}

struct xpool *link_get_xpool_by_name(link_conn_xpool_t *pool, const char *name) {

}

struct xpool *link_get_xpool_by_hash(link_conn_xpool_t *pool, const char *name, const char* hash) {

}
