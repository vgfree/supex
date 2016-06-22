#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "link_cfg_parser.h"
#include "json.h"

struct link_cfg *link_get_cfg(struct json_object *type_cfg, const char *name) {
	struct json_object      *obj = NULL;
	struct json_object      *host_obj = NULL;
	struct json_object      *port_obj = NULL;
	struct json_object      *size_obj = NULL;

	char            *host = NULL;
	unsigned short  port = 0;
	unsigned short  size = 1;

	if (json_object_object_get_ex(type_cfg, name, &obj)) {
		if (json_object_object_get_ex(obj, "host", &host_obj)) {
			host = link_strdup(json_object_get_string(host_obj));
		}

		if (json_object_object_get_ex(obj, "port", &port_obj)) {
			port = (unsigned short)json_object_get_int(port_obj);
		}

		if (json_object_object_get_ex(obj, "size", &size_obj)) {
			size = (unsigned short)json_object_get_int(size_obj);
		}

		struct link_cfg *cfg = malloc(sizeof(*cfg));
		if (cfg == NULL) {
			return NULL;
		}

		cfg->host = host;
		cfg->port = port;
		cfg->size = size;
		return cfg;
	} else {
		return NULL;
	}
}

void link_free_cfg(struct link_cfg *cfg) {
	if (cfg) {
		if (cfg->host) {
			free(cfg->host);
		}

		free(cfg);
	}
}

struct link_hash_cfg *link_get_hash_cfg(struct json_object *type_cfg, const char *name) {
	struct json_object      *obj = NULL;
	struct json_object      *hash_obj = NULL;
	struct json_object      *nodes_obj = NULL;

	char                      *hash = NULL;
	struct link_hash_node     *nodes = NULL;
	struct link_hash_node     *node;

	if (json_object_object_get_ex(type_cfg, name, &obj)) {
		if (json_object_object_get_ex(obj, "hash", &hash_obj)) {
			hash = link_strdup(json_object_get_string(hash_obj));
		}

		int count = 0;

		if (json_object_object_get_ex(obj, "nodes", &nodes_obj)) {
			count = json_object_array_length(nodes_obj);
			int                     i;
			struct json_object      *itr_obj = NULL;
			struct json_object      *tmp_obj = NULL;

			nodes = malloc(sizeof(*nodes) * count);

			if (nodes == NULL) {
				return NULL;
			}

			for (i = 0; i < count; i++) {
				node = &nodes[i];
				itr_obj = json_object_array_get_idx(nodes_obj, i);

				tmp_obj = json_object_array_get_idx(itr_obj, 0);
				node->mode = link_strdup(json_object_get_string(tmp_obj));

				tmp_obj = json_object_array_get_idx(itr_obj, 1);
				node->mark = link_strdup(json_object_get_string(tmp_obj));

				tmp_obj = json_object_array_get_idx(itr_obj, 2);
				node->host = link_strdup(json_object_get_string(tmp_obj));

				tmp_obj = json_object_array_get_idx(itr_obj, 3);
				node->port = (unsigned short)json_object_get_int(tmp_obj);

				tmp_obj = json_object_array_get_idx(itr_obj, 4);
				node->vnode = (unsigned short)json_object_get_int(tmp_obj);
			}
		}

		struct link_hash_cfg *cfg = malloc(sizeof(*cfg));

		if (cfg == NULL) {
			return NULL;
		}

		cfg->hash = hash;
		cfg->count = count;
		cfg->nodes = nodes;
		return cfg;
	} else {
		return NULL;
	}
}

void link_free_hash_cfg(struct link_hash_cfg *cfg) {
	if (cfg) {
		if (cfg->hash) {
			free(cfg->hash);
		}

		free(cfg);
	}
}

struct link_mysql_cfg *link_get_mysql_cfg(struct json_object *mysql_cfg, const char *name) {
	struct json_object      *obj = NULL;
	struct json_object      *host_obj = NULL;
	struct json_object      *port_obj = NULL;
	struct json_object      *size_obj = NULL;
	struct json_object      *database_obj = NULL;
	struct json_object      *user_obj = NULL;
	struct json_object      *password_obj = NULL;

	char            *host = NULL;
	char            *database = NULL;
	char            *user = NULL;
	char            *password = NULL;
	unsigned short  port = 0;
	unsigned short  size = 1;

	if (json_object_object_get_ex(mysql_cfg, name, &obj)) {
		if (json_object_object_get_ex(obj, "host", &host_obj)) {
			host = link_strdup(json_object_get_string(host_obj));
		}

		if (json_object_object_get_ex(obj, "port", &port_obj)) {
			port = (unsigned short)json_object_get_int(port_obj);
		}

		if (json_object_object_get_ex(obj, "size", &size_obj)) {
			size = (unsigned short)json_object_get_int(size_obj);
		}

		if (json_object_object_get_ex(obj, "database", &database_obj)) {
			database = link_strdup(json_object_get_string(database_obj));
		}

		if (json_object_object_get_ex(obj, "user", &user_obj)) {
			user = link_strdup(json_object_get_string(user_obj));
		}

		if (json_object_object_get_ex(obj, "password", &password_obj)) {
			password = link_strdup(json_object_get_string(password_obj));
		}

		struct link_mysql_cfg *cfg = malloc(sizeof(*cfg));

		if (cfg == NULL) {
			return NULL;
		}

		cfg->host = host;
		cfg->port = port;
		cfg->size = size;
		cfg->database = database;
		cfg->user = user;
		cfg->password = password;
		return cfg;
	} else {
		return NULL;
	}
}

void link_free_mysql_cfg(struct link_mysql_cfg *cfg) {
	if (cfg) {
		if (cfg->host) {
			free(cfg->host);
		}

		if (cfg->database) {
			free(cfg->database);
		}

		if (cfg->user) {
			free(cfg->user);
		}

		if (cfg->password) {
			free(cfg->password);
		}

		free(cfg);
	}
}
