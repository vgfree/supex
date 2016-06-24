#include <string.h>

#include "link_xpool.h"
#include "link_cfg_parser.h"
#include "link_hashmap.h"
#include "custom_hash.h"
#include "conhash.h"

static struct json_object *json_cfg;
static link_hashmap_t *redis_hashmap;
static link_hashmap_t *html_hashmap;
static link_hashmap_t *redis_hash_hashmap;
static link_hashmap_t *html_hash_hashmap;

typedef struct {
  struct conhash_s      *root;
  struct node_s         *nodes;
  link_hashmap_t        *hashmap;
  int                   count;
  enum link_hash_type   type;
} link_hash_pool_t;

typedef struct {
  struct xpool    *cpool;
  char            *host;
  unsigned short  port;
} link_xpool_t;

static void free_hash_pool(void *p) {
  if (p != NULL) {
    link_hash_pool_t *tmp = p;
    if (tmp->root) conhash_fini(tmp->root);
    if (tmp->nodes) free(tmp->nodes);
    if (tmp->hashmap) link_hashmap_destory(tmp->hashmap);
    free(p);
  }
}

static void free_xpool(void *p) {
  if (p != NULL) {
    link_xpool_t *tmp = p;
    conn_xpool_destroy(tmp->host, tmp->port);
    free(tmp->host);
    free(p);
  }
}

int link_xpool_init(const char *filename) {
  struct json_object *cfg = json_object_from_file(filename);
  if (cfg == NULL) {
	  x_printf(E, "Invalid config file :%s", filename);
    return -1;
  }

  link_hashmap_t *redis_hashmap = link_hashmap_create(LINK_64_SHIFT, strncmp, link_hash, free, free);
  if (redis_hashmap == NULL) {
	  x_printf(E, "No memory for redis hashmap");
    return -1;
  }

  link_hashmap_t *html_hashmap = link_hashmap_create(LINK_64_SHIFT, strncmp, link_hash, free, free);
  if (html_hashmap == NULL) {
	  x_printf(E, "No memory for html hashmap");
    return -1;
  }

  link_hashmap_t *redis_hash_hashmap = link_hashmap_create(LINK_64_SHIFT, strncmp, link_hash, free, free_hash_pool);
  if (redis_hash_hashmap == NULL) {
	  x_printf(E, "No memory for redis hash hashmap");
    return -1;
  }

  link_hashmap_t *html_hash_hashmap = link_hashmap_create(LINK_64_SHIFT, strncmp, link_hash, free, free_hash_pool);
  if (html_hash_hashmap == NULL) {
	  x_printf(E, "No memory for html hash hashmap");
    return -1;
  }

  return 0;
}

void link_xpool_destory(void) {
  if (json_cfg) json_object_put(json_cfg);
  if (redis_hashmap) link_hashmap_destory(redis_hashmap);
  if (redis_hash_hashmap) link_hashmap_destory(redis_hash_hashmap);
  if (html_hashmap) link_hashmap_destory(html_hashmap);
  if (html_hash_hashmap) link_hashmap_destory(html_hash_hashmap);
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
		  link_free_cfg(cfg);
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
		  link_free_cfg(cfg);
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

/**
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
**/

struct xpool *link_get_redis_xpool_by_conhash(const char *name, const char* hash) {
  char temp[64];
  size_t len = strlen(name);

  struct xpool *cpool;
  const struct node_s *node;
  link_xpool_t *pool;
  link_hash_pool_t *hpool = link_hashmap_get_pointer(redis_hash_hashmap, name, len);
  if (hpool != NULL) {
    if (hpool->type != LINK_CONSISTENT_HASH) {
	    x_printf(E, "hash type must be consistent");
      return NULL;
    }

    node = conhash_lookup(hpool->root, hash);
    if (node != NULL) {
      pool = link_hashmap_get_pointer(hpool->hashmap, node->iden, strlen(node->iden));
      return pool->cpool;
    }
  }

	struct json_object *redis_cfg = NULL;
  struct link_hash_cfg *cfg;
  struct node_s *nodes;
  if (json_object_object_get_ex(json_cfg, "redis", &redis_cfg)) {
		cfg = link_get_hash_cfg(redis_cfg, name);
    if (cfg == NULL) {
	    x_printf(E, "Can't get redis.%s from config", name);
      return NULL;
    }

    hpool = malloc(sizeof(link_hash_pool_t));
    if (hpool == NULL) {
		  link_free_hash_cfg(cfg);
	    x_printf(E, "No memory for hash pool");
      return NULL;
    }

    int count = cfg->count;
    nodes = malloc(sizeof(struct node_s) * count);
    if (nodes == NULL) {
	    x_printf(E, "No memory for nodes");
      goto ERROR;
    }

    hpool->nodes = nodes;

    link_hashmap_t *hashmap = link_hashmap_create(LINK_64_SHIFT, strncmp, link_hash, free, free_xpool);
    if (hashmap == NULL) {
	    x_printf(E, "No memory for hashmap");
      goto ERROR;
    }

    hpool->hashmap = hashmap;

    struct conhash_s *conhash = conhash_init(NULL);
    if (conhash == NULL) {
	    x_printf(E, "conhash init failed");
      goto ERROR;
    }

    int i, size;
    struct link_hash_node *hnode;
    for (i = 0; i < count; i++) {
      hnode = &cfg->nodes[i];
      
      cpool = conn_xpool_find(hnode->host, hnode->port);
      if (cpool == NULL) {
        /*@TODO hnode->size*/
        conn_xpool_init(hnode->host, hnode->port, 1, true);
        cpool = conn_xpool_find(hnode->host, hnode->port);
      }

      if (cpool == NULL) {
	      x_printf(E, "xpool init failed");
        goto ERROR;
      }

      pool = malloc(sizeof(link_xpool_t));
      if (pool == NULL) {
	      x_printf(E, "No memory for pool");
        goto ERROR;
      }

      pool->cpool = cpool;
      pool->host = link_strdup(hnode->host);
      pool->port = hnode->port;

      size = sprintf(temp, "%s:%d", hnode->host, hnode->port);
      conhash_set_node(&nodes[i], temp, hnode->vnode);
      conhash_add_node(conhash, &nodes[i]);

      char *pkey = link_strdup(temp);
      link_hashmap_set_pointer(hashmap, pkey, size, pool);
    }

    hpool->type == LINK_CONSISTENT_HASH;

    char *key = link_strdup(name);
    link_hashmap_set_pointer(redis_hash_hashmap, key, len, hpool);

    node = conhash_lookup(hpool->root, hash);
    if (node != NULL) {
      pool = link_hashmap_get_pointer(hpool->hashmap, node->iden, strlen(node->iden));
      return pool->cpool;
    }
  } else {
	  x_printf(E, "Can't get html from config");
    return NULL;
  }

ERROR:
  free_hash_pool(hpool);
	link_free_hash_cfg(cfg);
  return NULL;
}

struct xpool *link_get_html_xpool_by_conhash(const char *name, const char* hash) {
  char temp[64];
  size_t len = strlen(name);

  struct xpool *cpool;
  const struct node_s *node;
  link_xpool_t *pool;
  link_hash_pool_t *hpool = link_hashmap_get_pointer(html_hash_hashmap, name, len);
  if (hpool != NULL) {
    if (hpool->type != LINK_CONSISTENT_HASH) {
	    x_printf(E, "hash type must be consistent");
      return NULL;
    }

    node = conhash_lookup(hpool->root, hash);
    if (node != NULL) {
      pool = link_hashmap_get_pointer(hpool->hashmap, node->iden, strlen(node->iden));
      return pool->cpool;
    }
  }

	struct json_object *html_cfg = NULL;
  struct link_hash_cfg *cfg;
  struct node_s *nodes;
  if (json_object_object_get_ex(json_cfg, "html", &html_cfg)) {
		cfg = link_get_hash_cfg(html_cfg, name);
    if (cfg == NULL) {
	    x_printf(E, "Can't get html.%s from config", name);
      return NULL;
    }

    hpool = malloc(sizeof(link_hash_pool_t));
    if (hpool == NULL) {
		  link_free_hash_cfg(cfg);
	    x_printf(E, "No memory for hash pool");
      return NULL;
    }

    int count = cfg->count;
    nodes = malloc(sizeof(struct node_s) * count);
    if (nodes == NULL) {
	    x_printf(E, "No memory for nodes");
      goto ERROR;
    }

    hpool->nodes = nodes;

    link_hashmap_t *hashmap = link_hashmap_create(LINK_64_SHIFT, strncmp, link_hash, free, free_xpool);
    if (hashmap == NULL) {
	    x_printf(E, "No memory for hashmap");
      goto ERROR;
    }

    hpool->hashmap = hashmap;

    struct conhash_s *conhash = conhash_init(NULL);
    if (conhash == NULL) {
	    x_printf(E, "conhash init failed");
      goto ERROR;
    }

    int i, size;
    struct link_hash_node *hnode;
    for (i = 0; i < count; i++) {
      hnode = &cfg->nodes[i];
      
      cpool = conn_xpool_find(hnode->host, hnode->port);
      if (cpool == NULL) {
        /*@TODO hnode->size*/
        conn_xpool_init(hnode->host, hnode->port, 1, true);
        cpool = conn_xpool_find(hnode->host, hnode->port);
      }

      if (cpool == NULL) {
	      x_printf(E, "xpool init failed");
        goto ERROR;
      }

      pool = malloc(sizeof(link_xpool_t));
      if (pool == NULL) {
	      x_printf(E, "No memory for pool");
        goto ERROR;
      }

      pool->cpool = cpool;
      pool->host = link_strdup(hnode->host);
      pool->port = hnode->port;

      size = sprintf(temp, "%s:%d", hnode->host, hnode->port);
      conhash_set_node(&nodes[i], temp, hnode->vnode);
      conhash_add_node(conhash, &nodes[i]);

      char *pkey = link_strdup(temp);
      link_hashmap_set_pointer(hashmap, pkey, size, pool);
    }

    hpool->type == LINK_CONSISTENT_HASH;

    char *key = link_strdup(name);
    link_hashmap_set_pointer(html_hash_hashmap, key, len, hpool);

    node = conhash_lookup(hpool->root, hash);
    if (node != NULL) {
      pool = link_hashmap_get_pointer(hpool->hashmap, node->iden, strlen(node->iden));
      return pool->cpool;
    }
  } else {
	  x_printf(E, "Can't get html from config");
    return NULL;
  }

ERROR:
  free_hash_pool(hpool);
	link_free_hash_cfg(cfg);
  return NULL;
}
