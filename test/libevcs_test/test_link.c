#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "link_cfg_parser.h"
#include "json.h"

void main(void) {
  struct json_object *cfg = json_object_from_file("link.json");

  if (cfg == NULL) {
    printf("json file load error\n");
  }

  struct json_object *redis_cfg = NULL;
	if (json_object_object_get_ex(cfg, "redis", &redis_cfg)) {
    struct link_redis_cfg *public = link_get_redis_cfg(redis_cfg, "public");

    printf("public->host: %s\n", public->host);
    printf("public->port: %d\n", public->port);
    printf("public->size: %d\n", public->size);
    printf("=============================================\n");

    link_free_redis_cfg(public);

    struct link_redis_cfg *private = link_get_redis_cfg(redis_cfg, "private");

    printf("private->host: %s\n", private->host);
    printf("private->port: %d\n", private->port);
    printf("private->size: %d\n", private->size);
    printf("=============================================\n");

    link_free_redis_cfg(private);

    struct link_redis_cfg *weibo = link_get_redis_cfg(redis_cfg, "weibo");

    printf("weibo->host: %s\n", weibo->host);
    printf("weibo->port: %d\n", weibo->port);
    printf("weibo->size: %d\n", weibo->size);
    printf("=============================================\n");

    link_free_redis_cfg(weibo);

    int i;
    struct link_redis_hash_cfg *tsdb_hash = link_get_redis_hash_cfg(redis_cfg, "tsdb_hash");
    printf("tsdb_hash->hash: %s\n", tsdb_hash->hash);
    printf("tsdb_hash->count: %d\n", tsdb_hash->count);
    
    int count = tsdb_hash->count;
    for (i = 0; i < count; i++) {
      printf("tsdb_hash->nodes[%d].mode: %s\n", i, tsdb_hash->nodes[i].mode);
      printf("tsdb_hash->nodes[%d].mark: %s\n", i, tsdb_hash->nodes[i].mark);
      printf("tsdb_hash->nodes[%d].host: %s\n", i, tsdb_hash->nodes[i].host);
      printf("tsdb_hash->nodes[%d].port: %d\n", i, tsdb_hash->nodes[i].port);
      printf("tsdb_hash->nodes[%d].vnode: %d\n", i, tsdb_hash->nodes[i].vnode);
    }
    printf("=============================================\n");

    link_free_redis_hash_cfg(tsdb_hash);

    struct link_redis_hash_cfg *url_hash = link_get_redis_hash_cfg(redis_cfg, "url_hash");
    printf("url_hash->hash: %s\n", url_hash->hash);
    printf("url_hash->count: %d\n", url_hash->count);
    
    count = url_hash->count;
    for (i = 0; i < count; i++) {
      printf("url_hash->nodes[%d].mode: %s\n", i, url_hash->nodes[i].mode);
      printf("url_hash->nodes[%d].mark: %s\n", i, url_hash->nodes[i].mark);
      printf("url_hash->nodes[%d].host: %s\n", i, url_hash->nodes[i].host);
      printf("url_hash->nodes[%d].port: %d\n", i, url_hash->nodes[i].port);
      printf("url_hash->nodes[%d].vnode: %d\n", i, url_hash->nodes[i].vnode);
    }
    printf("=============================================\n");

    link_free_redis_hash_cfg(url_hash);
  } else {
    printf("no redis\n");
  }

  struct json_object *mysql_cfg = NULL;
	if (json_object_object_get_ex(cfg, "mysql", &mysql_cfg)) {
    struct link_mysql_cfg *app_url = link_get_mysql_cfg(mysql_cfg, "app_url");

    printf("app_url->host: %s\n", app_url->host);
    printf("app_url->port: %d\n", app_url->port);
    printf("app_url->size: %d\n", app_url->size);
    printf("app_url->database: %s\n", app_url->database);
    printf("app_url->user: %s\n", app_url->user);
    printf("app_url->password: %s\n", app_url->password);
    printf("=============================================\n");

    link_free_mysql_cfg(app_url);

    struct link_mysql_cfg *app_newStatus = link_get_mysql_cfg(mysql_cfg, "app_newStatus");

    printf("app_newStatus->host: %s\n", app_newStatus->host);
    printf("app_newStatus->port: %d\n", app_newStatus->port);
    printf("app_newStatus->size: %d\n", app_newStatus->size);
    printf("app_newStatus->database: %s\n", app_newStatus->database);
    printf("app_newStatus->user: %s\n", app_newStatus->user);
    printf("app_newStatus->password: %s\n", app_newStatus->password);
    printf("=============================================\n");

    link_free_mysql_cfg(app_newStatus);
  } else {
    printf("no mysql\n");
  }

  json_object_put(cfg);
}
