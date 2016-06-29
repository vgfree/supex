#include "json.h"

#include "utils.h"

#include "timport_cfg.h"

#define MIN_DELAY_TIME  3600
#define MIN_EXPIRE_TIME 1800

void read_timport_cfg(struct timport_cfg_file *p_cfg, char *name)
{
	int                     i = 0;
	int                     array_len = 0;
	const char              *str_val = NULL;
	struct json_object      *obj = NULL, *tmp_obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	printf("fileName = %s\n", name);

	if (cfg == NULL) {
		printf("cfg is NULL!\n");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "delay_time", &obj)) {
		p_cfg->delay_time = json_object_get_int(obj);
		printf("(delay_time) found, delay_time = %d\n", p_cfg->delay_time);
	}

	if (json_object_object_get_ex(cfg, "redis", &obj)) {
		array_len = json_object_array_length(obj);

		if (array_len <= 0) {
			printf("(redis) array length err");
			goto fail;
		}

		NewArray0(array_len, p_cfg->redis);

		if (NULL == p_cfg->redis) {
			printf("(redis) NewArray0 err");
			goto fail;
		}

		p_cfg->redis_cnt = array_len;

		for (i = 0; i < array_len; ++i) {
			struct json_object *rds_obj = json_object_array_get_idx(obj, i);

			if (json_object_object_get_ex(rds_obj, "host", &tmp_obj)) {
				str_val = json_object_get_string(tmp_obj);
				p_cfg->redis[i].host = x_strdup(str_val);
			} else {
				printf("(redis) no (host)");
				goto fail;
			}

			if (json_object_object_get_ex(rds_obj, "port", &tmp_obj)) {
				p_cfg->redis[i].port = json_object_get_int(tmp_obj);
			} else {
				printf("(redis) no (port)");
				goto fail;
			}

			p_cfg->redis[i].idx = i;
		}
	} else {
		printf("(redis) not found");
		goto fail;
	}

	json_object_put(cfg);

	return;

fail:

	if (cfg) {
		json_object_put(cfg);
	}

	printf("invalid config file :%s", name);
	exit(EXIT_FAILURE);
}

