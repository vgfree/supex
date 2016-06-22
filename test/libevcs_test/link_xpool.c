#include <string.h>

#include "link_xpool.h"
#include "link_cfg_parser.h"
#include "link_hashmap.h"
#include "custom_hash.h"
#include "conhash.h"

static struct json_object *json_cfg;
static link_hashmap_t *redis_hashmap;
static link_hashmap_t *html_hashmap;
static link_hashmap_t *ip_hashmap;

enum link_hash_type {
  LINK_CUSTEOMER_HASH = 0,
  LINK_CONSISTENT_HASH
};

typedef struct {
  struct xpool          **nodes;
  enum link_hash_type   type;
} link_hash_pool_t;

int link_xpool_init(const char *filename) {
  struct json_object *cfg = json_object_from_file(filename);
  if (cfg == NULL) {
	  x_printf(E, "Invalid config file :%s", filename);
    return -1;
  }

  link_hashmap_t *redis_hashmap = link_hashmap_create(LINK_128_SHIFT, strncmp, link_hash);
  if (redis_hashmap == NULL) {
	  x_printf(E, "No memory for redis hashmap");
    return -1;
  }

  link_hashmap_t *html_hashmap = link_hashmap_create(LINK_128_SHIFT, strncmp, link_hash);
  if (html_hashmap == NULL) {
	  x_printf(E, "No memory for html hashmap");
    return -1;
  }

  link_hashmap_t *ip_hashmap = link_hashmap_create(LINK_256_SHIFT, strncmp, link_hash);
  if (ip_hashmap == NULL) {
	  x_printf(E, "No memory for ip hashmap");
    return -1;
  }

  return 0;
}

struct xpool *link_get_redis_xpool_by_name(const char *name) {
  size_t len = strlen(name);
  struct xpool *cpool = link_hashmap_get_pointer(redis_hashmap, name, len);
  if (cpool != NULL) return cpool;

	struct json_object *redis_cfg = NULL;
	if (json_object_object_get_ex(json_cfg, "redis", &redis_cfg)) {
		struct link_cfg *cfg = link_get_cfg(redis_cfg, name);
    if (cfg == NULL) {
	    x_printf(E, "Can't get redis.%s from config", name);
      return NULL;
    }

    cpool = conn_xpool_find(cfg->host, cfg->port);
    if (cpool == NULL) {
      conn_xpool_init(cfg->host, cfg->port, cfg->size, true);
      cpool = conn_xpool_find(cfg->host, cfg->port);
    }

    if (cpool == NULL) {
	    x_printf(E, "Init xpool failed");
      return NULL;
    }

    char *key = link_strdup(name);
    link_hashmap_set_pointer(redis_hashmap, key, len, cpool); 

		link_free_cfg(cfg);
    return cpool;
  } else {
	  x_printf(E, "Can't get redis from config");
    return NULL;
  }
}

struct xpool *link_get_html_xpool_by_name(const char *name) {
  size_t len = strlen(name);
  struct xpool *cpool = link_hashmap_get_pointer(html_hashmap, name, len);
  if (cpool != NULL) return cpool;

	struct json_object *html_cfg = NULL;
	if (json_object_object_get_ex(json_cfg, "html", &html_cfg)) {
		struct link_cfg *cfg = link_get_cfg(html_cfg, name);
    if (cfg == NULL) {
	    x_printf(E, "Can't get html.%s from config", name);
      return NULL;
    }

    cpool = conn_xpool_find(cfg->host, cfg->port);
    if (cpool == NULL) {
      conn_xpool_init(cfg->host, cfg->port, cfg->size, true);
      cpool = conn_xpool_find(cfg->host, cfg->port);
    }

    if (cpool == NULL) {
	    x_printf(E, "Init xpool failed");
      return NULL;
    }

    char *key = link_strdup(name);
    link_hashmap_set_pointer(html_hashmap, key, len, cpool); 

		link_free_cfg(cfg);
    return cpool;
  } else {
	  x_printf(E, "Can't get html from config");
    return NULL;
  }
}

struct xpool *link_get_redis_xpool_by_hash(const char *name, const char* hash) {
  size_t len = strlen(name);
  struct xpool *cpool = link_hashmap_get_pointer(redis_hashmap, name, len);
  if (cpool != NULL) return cpool;

}

struct xpool *link_get_html_xpool_by_hash(const char *name, const char* hash) {
  size_t len = strlen(name);
  struct xpool *cpool = link_hashmap_get_pointer(html_hashmap, name, len);
  if (cpool != NULL) return cpool;

}

struct xpool *link_get_redis_xpool_by_conhash(const char *name, const char* hash) {
  size_t len = strlen(name);
  struct xpool *cpool = link_hashmap_get_pointer(redis_hashmap, name, len);
  if (cpool != NULL) {
    return cpool;
  }
}

struct xpool *link_get_html_xpool_by_conhash(const char *name, const char* hash) {
  size_t len = strlen(name);
  struct xpool *cpool = NULL;
  link_hash_pool_t *hpool = link_hashmap_get_pointer(html_hashmap, name, len);
  if (hpool != NULL) {
    if (hpool->type != LINK_CONSISTENT_HASH) {
	    x_printf(E, "hash type must be consistent");
      return NULL;
    }



    return cpool;
  }

    struct conhash_s *conhash = conhash_init(NULL);
    if (conhash == NULL) {
	    x_printf(E, "conhash init failed");
      return NULL;
    }
}
