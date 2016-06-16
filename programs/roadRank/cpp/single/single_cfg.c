#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "single_cfg.h"
#include "json.h"

#if 0
void load_single_cfg_file(struct single_cfg_file *p_cfg, char *name)
{
	// const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "kv_cache_count", &obj)) {
		p_cfg->kv_cache_count = (unsigned short)json_object_get_int(obj);
	} else { goto fail; }

	/*if (json_object_object_get_ex(cfg, "log_path", &obj)) {
	 *        str_val = json_object_get_string(obj);
	 *        p_cfg->log_path = x_strdup(str_val);
	 *   } else { goto fail; }
	 */

	if (json_object_object_get_ex(cfg, "gpscount_limit", &obj)) {
		p_cfg->gpscount_limit = (unsigned short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "expire_time", &obj)) {
		p_cfg->expire_time = (unsigned short)json_object_get_int(obj);
	} else { goto fail; }

	json_object_put(cfg);

	return;

fail:

	if (cfg) {
		json_object_put(cfg);
	}

	x_printf(E, "invalid config file :%s", name);
	exit(EXIT_FAILURE);
}
#endif /* if 0 */

bool fill_single_model(struct json_object *obj, char *obj_name, struct single_cfg_file *p_link)
{
	struct json_object      *sub_obj = NULL;
	struct json_object      *countlimit_obj = NULL;
	struct json_object      *kvcount_obj = NULL;
	struct json_object      *expire_obj = NULL;

	if (!obj || !obj_name || !p_link) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(obj, obj_name, &sub_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "road_match_limit", &countlimit_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "kv_cache_count", &kvcount_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "expire_time", &expire_obj)) {
		goto fill_fail;
	}

	unsigned short  gpscount_limit = (short)json_object_get_int(countlimit_obj);
	unsigned short  kv_cache_count = (short)json_object_get_int(kvcount_obj);
	unsigned short  expire_time = (short)json_object_get_int(expire_obj);

	p_link->road_match_limit = gpscount_limit;
	p_link->kv_cache_count = kv_cache_count;
	p_link->expire_time = expire_time;

	return true;

fill_fail:
	x_printf(E, "cann't find %s \n", obj_name);
	return false;
}

