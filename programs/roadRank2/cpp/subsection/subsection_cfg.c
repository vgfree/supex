#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "subsection_cfg.h"
#include "json.h"

#if 0
void load_subsec_cfg_file(struct subsec_cfg_file *p_cfg, char *name)
{
	// const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "kv_cache_count", &obj)) {
		p_cfg->kv_cache_count = (unsigned short)json_object_get_int(obj);
	} else {
		goto fail;
	}

	/*if (json_object_object_get_ex(cfg, "log_path", &obj)) {
	 *        str_val = json_object_get_string(obj);
	 *        p_cfg->log_path = x_strdup(str_val);
	 *   } else { goto fail; }
	 */

	if (json_object_object_get_ex(cfg, "gpscount_limit", &obj)) {
		p_cfg->gpscount_limit = (unsigned short)json_object_get_int(obj);
	} else {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "expire_time", &obj)) {
		p_cfg->expire_time = (unsigned short)json_object_get_int(obj);
	} else {
		goto fail;
	}

	json_object_put(cfg);

	return;

fail:

	if (cfg) {
		json_object_put(cfg);
	}

	x_printf(E, "invalid config file :%s", name);
	exit(EXIT_FAILURE);
}
#endif	/* if 0 */
bool fill_subsec_model(struct json_object *obj, char *obj_name, struct subsec_cfg_file *p_link)
{
	struct json_object      *sub_obj = NULL;
	struct json_object      *roadcount_obj = NULL;
	struct json_object      *countlimit_obj = NULL;
	struct json_object      *replace_limit_h_obj = NULL;
	struct json_object      *kvcount_obj = NULL;
	struct json_object      *expire_obj = NULL;
	struct json_object      *merged_obj = NULL;
	struct json_object      *merged_2obj = NULL;
	struct json_object      *init_max_obj = NULL;

	if (!obj || !obj_name || !p_link) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(obj, obj_name, &sub_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "road_match_limit", &roadcount_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "replace_limit_l", &countlimit_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "replace_limit_h", &replace_limit_h_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "kv_cache_count", &kvcount_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "expire_time", &expire_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "merged_speed_l", &merged_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "merged_speed_h", &merged_2obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "init_max", &init_max_obj)) {
		goto fill_fail;
	}

	unsigned short  roadcount_limit = (short)json_object_get_int(roadcount_obj);
	unsigned short  replace_limit = (short)json_object_get_int(countlimit_obj);
	unsigned short  replace_limit_h = (short)json_object_get_int(replace_limit_h_obj);
	unsigned short  kv_cache_count = (short)json_object_get_int(kvcount_obj);
	unsigned short  expire_time = (short)json_object_get_int(expire_obj);
	unsigned short  merged_speed_l = (short)json_object_get_int(merged_obj);
	unsigned short  merged_speed_h = (short)json_object_get_int(merged_2obj);
	unsigned short  init_max = (short)json_object_get_int(init_max_obj);

	p_link->road_match_limit = roadcount_limit;
	p_link->replace_limit_l = replace_limit;
	p_link->replace_limit_h = replace_limit_h;
	p_link->kv_cache_count = kv_cache_count;
	p_link->expire_time = expire_time;
	p_link->merged_speed_l = merged_speed_l;
	p_link->merged_speed_h = merged_speed_h;
	p_link->init_max = init_max;

	return true;

fill_fail:
	x_printf(E, "cann't find %s \n", obj_name);
	return false;
}

