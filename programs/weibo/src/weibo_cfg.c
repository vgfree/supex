#include <json.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "weibo_cfg.h"
#include "utils.h"

void read_weibo_cfg(struct weibo_cfg_file *p_cfg, char *name)
{
	int                     i = 0;
	int                     idx = 0;
	int                     add = 0;
	const char              *str_val = NULL;
	struct json_object      *tmp = NULL;
	struct json_object      *ary = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	/*weibo_store*/
	if (json_object_object_get_ex(cfg, "weibo_store", &ary)) {
		add = json_object_array_length(ary);
		p_cfg->weibo_count = add;
		assert(p_cfg->weibo_count <= MAX_LINK_INDEX);

		for (i = 0; i < add; i++) {
			tmp = json_object_array_get_idx(ary, i);

			if (json_object_object_get_ex(tmp, "port", &obj)) {
				p_cfg->weibo_store[i].port = (short)json_object_get_int(obj);
			} else { goto fail; }

			if (json_object_object_get_ex(tmp, "host", &obj)) {
				str_val = json_object_get_string(obj);
				p_cfg->weibo_store[i].host = x_strdup(str_val);
			} else { goto fail; }
		}
	} else { goto fail; }

	/*weibo_static*/
	if (json_object_object_get_ex(cfg, "weibo_static", &ary)) {
		add = json_object_array_length(ary);
		p_cfg->static_count = add;
		assert(p_cfg->static_count <= MAX_LINK_INDEX);

		for (i = 0; i < add; i++) {
			tmp = json_object_array_get_idx(ary, i);

			if (json_object_object_get_ex(tmp, "port", &obj)) {
				p_cfg->weibo_static[i].port = (short)json_object_get_int(obj);
			} else { goto fail; }

			if (json_object_object_get_ex(tmp, "host", &obj)) {
				str_val = json_object_get_string(obj);
				p_cfg->weibo_static[i].host = x_strdup(str_val);
			} else { goto fail; }
		}
	} else { goto fail; }

	/*weidb*/
	if (json_object_object_get_ex(cfg, "weidb", &ary)) {
		add = json_object_array_length(ary);
		p_cfg->weidb_count = add;
		assert(p_cfg->weidb_count <= MAX_LINK_INDEX);

		for (i = 0; i < add; i++) {
			tmp = json_object_array_get_idx(ary, i);

			if (json_object_object_get_ex(tmp, "port", &obj)) {
				p_cfg->weidb[i].port = (short)json_object_get_int(obj);
			} else { goto fail; }

			if (json_object_object_get_ex(tmp, "host", &obj)) {
				str_val = json_object_get_string(obj);
				p_cfg->weidb[i].host = x_strdup(str_val);
			} else { goto fail; }
		}
	} else { goto fail; }

	return;

fail:
	x_printf(E, "invalid weibo config file :%s", name);
	exit(0);
}

