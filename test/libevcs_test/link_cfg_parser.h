/* @author: qianye@mirrtalk.com */

#ifndef LINK_CFG_PARSER_H
#define LINK_CFG_PARSER_H

#include "json.h"

struct link_redis_cfg {
  char            *host;
  unsigned short   port;
  unsigned short   size;
};

struct link_redis_hash_node {
  char            *mode;
  char            *mark;
  char            *host;
  unsigned short   port;
  unsigned short   vnode;
};

struct link_redis_hash_cfg {
  char                          *hash;
  int                           count;
  struct link_redis_hash_node   *nodes;
};

struct link_mysql_cfg {
  char            *host;
  char            *database;
  char            *user;
  char            *password;
  unsigned short   port;
  unsigned short   size;
};

struct link_redis_cfg *link_get_redis_cfg(struct json_object *redis_cfg, const char *name);
void link_free_redis_cfg(struct link_redis_cfg *cfg);

struct link_redis_hash_cfg *link_get_redis_hash_cfg(struct json_object *redis_cfg, const char *name);
void link_free_redis_hash_cfg(struct link_redis_hash_cfg *cfg);

struct link_mysql_cfg *link_get_mysql_cfg(struct json_object *mysql_cfg, const char *name);
void link_free_mysql_cfg(struct link_mysql_cfg *cfg);

#endif /* LINK_CFG_PARSER_H */
