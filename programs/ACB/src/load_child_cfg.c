#include <string.h>
#include <assert.h>

#include "json.h"
#include "load_child_cfg.h"

void load_child_cfg_file(struct child_cfg_list *p_cfg, char *name)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	// if (json_object_object_get_ex(cfg, "crzpt_worker_counts", &obj)) {
	//        p_cfg->worker_counts = (short)json_object_get_int(obj);
	// } else goto fail;

	if (json_object_object_get_ex(cfg, "child_online", &obj)) {
		p_cfg->app_counts = MIN(json_object_array_length(obj), MAX_APP_COUNTS);

		int                     i = 0;
		struct json_object      *itr_obj = NULL;
		memset(p_cfg->app_names, 0, MAX_APP_COUNTS * (MAX_FILE_NAME_SIZE));
		memset(p_cfg->app_msmqs, 0, MAX_APP_COUNTS * (MAX_FILE_NAME_SIZE));

		for (i = 0; i < p_cfg->app_counts; i++) {
			itr_obj = json_object_array_get_idx(obj, i);
			str_val = json_object_get_string(itr_obj);
			strncpy(&p_cfg->app_names[i][0], str_val, MIN(strlen(str_val), MAX_FILE_NAME_SIZE - 1));
			struct json_object      *redis_node = NULL;
			struct json_object      *redis_host = NULL;
			struct json_object      *redis_port = NULL;
			json_object_object_get_ex(cfg, p_cfg->app_names[i], &redis_node);
			json_object_object_get_ex(redis_node, "host", &redis_host);
			json_object_object_get_ex(redis_node, "port", &redis_port);

			const char      *p_host = json_object_get_string(redis_host);
			unsigned short  s_port = (unsigned short)json_object_get_int(redis_port);

			strncpy(p_cfg->app_redis_host[i], p_host, MIN(strlen(p_host), MAX_FILE_NAME_SIZE - 1));
			p_cfg->app_redis_port[i] = s_port;

			p_cfg->app_msmqs[i][0] = '/';
			strncpy(&p_cfg->app_msmqs[i][1], str_val, MIN(strlen(str_val), MAX_FILE_NAME_SIZE - 2));
		}
	} else { goto fail; }

	return;

fail:
	x_printf(D, "invalid config file :%s\n", name);
	exit(EXIT_FAILURE);
}

