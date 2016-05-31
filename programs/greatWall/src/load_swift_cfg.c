#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "load_swift_cfg.h"
#include "json.h"

static void init_swift_cfg(struct swift_cfg_file *p_cfg)
{
	assert(p_cfg);
	memset(p_cfg, 0, sizeof(*p_cfg));
}

static void copy_swift_cfg(struct swift_cfg_file *dest, struct swift_cfg_file *src)
{
	assert(dest);
	assert(src);

	if (src->log_path) {
		dest->log_path = x_strdup(src->log_path);
	}

	if (src->log_path) {
		dest->log_path = x_strdup(src->log_path);
	}

	// #ifdef USE_HTTP_PROTOCOL

	// #endif
}

static void free_swift_cfg(struct swift_cfg_file *p_cfg)
{
	assert(p_cfg);

	Free(p_cfg->log_file);
	Free(p_cfg->log_path);
	// #ifdef USE_HTTP_PROTOCOL
	//        Free(p_cfg->api_apply);
	//        Free(p_cfg->api_fetch);
	//        Free(p_cfg->api_merge);
	// #endif
}

bool load_swift_cfg_file(struct swift_cfg_file *p_cfg, char *name)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = NULL;

	init_swift_cfg(p_cfg);

	cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "swift_port", &obj)) {
		p_cfg->srv_port = json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "swift_worker_counts", &obj)) {
		p_cfg->worker_counts = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "swift_tasker_counts", &obj)) {
		p_cfg->tasker_counts = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "swift_protocol", &obj)) {
		str_val = json_object_get_string(obj);

		if (strncmp(str_val, "http", 4) == 0) {
			p_cfg->ptype = USE_HTTP_PROTO;
		} else {
			p_cfg->ptype = USE_REDIS_PROTO;
		}
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "max_req_size", &obj)) {
		p_cfg->max_req_size = json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_path", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->log_path = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_file", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->log_file = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_level", &obj)) {
		p_cfg->log_level = json_object_get_int(obj);
	} else { goto fail; }

	if (p_cfg->ptype == USE_HTTP_PROTO) {
		if (json_object_object_get_ex(cfg, "api_apply", &obj)) {
			p_cfg->api_counts++;
			str_val = json_object_get_string(obj);
			p_cfg->api_apply = x_strdup(str_val);
		}

		if (json_object_object_get_ex(cfg, "api_fetch", &obj)) {
			p_cfg->api_counts++;
			str_val = json_object_get_string(obj);
			p_cfg->api_fetch = x_strdup(str_val);
		}

		if (json_object_object_get_ex(cfg, "api_merge", &obj)) {
			p_cfg->api_counts++;
			str_val = json_object_get_string(obj);
			p_cfg->api_merge = x_strdup(str_val);
		}

		if (json_object_object_get_ex(cfg, "api_custom", &obj)) {
			int add = json_object_array_length(obj);
			p_cfg->api_counts += add;

			assert(p_cfg->api_counts <= DIM(p_cfg->api_names));

			int                     i = 0;
			struct json_object      *itr_obj = NULL;

			for (i = 0; i < add; i++) {
				itr_obj = json_object_array_get_idx(obj, i);
				str_val = json_object_get_string(itr_obj);

				int len = snprintf(&p_cfg->api_names[i][0],
						sizeof(p_cfg->api_names[i]), "%s", str_val);

				assert(len <= sizeof(p_cfg->api_names[i]));
			}
		}
	}

	json_object_put(cfg);
	return true;

fail:

	if (cfg) {
		json_object_put(cfg);
	}

	x_printf(E, "invalid config file :%s", name);

	return false;
}

bool reload_swift_cfg_file(struct swift_cfg_file *p_cfg, const char *filename)
{
	struct swift_cfg_file   old = {};
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = NULL;

	assert(p_cfg);

	copy_swift_cfg(&old, p_cfg);

	free_swift_cfg(p_cfg);

	cfg = json_object_from_file(filename);

	if (cfg == NULL) {
		goto fail;
	}

	/*
	 * 需要重载属性
	 */
	if (json_object_object_get_ex(cfg, "log_path", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->log_path = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_file", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->log_file = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_level", &obj)) {
		p_cfg->log_level = json_object_get_int(obj);
	} else { goto fail; }

	/*
	 * do other something
	 */

	json_object_put(cfg);

	free_swift_cfg(&old);

	return true;

fail:

	if (cfg) {
		json_object_put(cfg);
	}

	free_swift_cfg(p_cfg);

	copy_swift_cfg(p_cfg, &old);

	free_swift_cfg(&old);

	x_printf(E, "invalid config file :%s", filename);
	return false;
}

