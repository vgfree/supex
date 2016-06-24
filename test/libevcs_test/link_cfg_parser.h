/* @author: qianye@mirrtalk.com */

#ifndef LINK_CFG_PARSER_H
#define LINK_CFG_PARSER_H

#include "json.h"

/*http redis*/
struct link_cfg {
	char            *host;
	unsigned short  port;
	unsigned short  size;
};

struct link_hash_node {
	char            *mode;
	char            *mark;
	char            *host;
	unsigned short  port;
	unsigned short  vnode;
};

enum link_hash_type {
  LINK_CUSTOMER_HASH = 0,
  LINK_CONSISTENT_HASH
};

struct link_hash_cfg {
	enum link_hash_type     type;
	int                     count;
	struct link_hash_node   *nodes;
};

struct link_mysql_cfg {
	char            *host;
	char            *database;
	char            *user;
	char            *password;
	unsigned short  port;
	unsigned short  size;
};

struct link_cfg *link_get_cfg(struct json_object *type_cfg, const char *name);

void link_free_cfg(struct link_cfg *cfg);

struct link_hash_cfg *link_get_hash_cfg(struct json_object *type_cfg, const char *name);

void link_free_hash_cfg(struct link_hash_cfg *cfg);

struct link_mysql_cfg *link_get_mysql_cfg(struct json_object *mysql_cfg, const char *name);

void link_free_mysql_cfg(struct link_mysql_cfg *cfg);

static char *link_strdup(const char *str) {
	if (str == NULL) return NULL;

	size_t len = strlen(str) + 1;
	char *copy = malloc(len);
	if (copy == NULL) return NULL;

	memcpy(copy, str, len);
	return copy;
}

#endif	/* LINK_CFG_PARSER_H */
