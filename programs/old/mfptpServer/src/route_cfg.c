#include <json.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "route_cfg.h"

static char *x_strdup(const char *src)
{
	if (src == NULL) {
		return NULL;
	}

	int     len = strlen(src);
	char    *out = calloc(len + 1, sizeof(char));
	assert(out);
	strcpy(out, src);
	return out;
}

void read_route_cfg(struct route_cfg_file *p_cfg, char *name)
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

	/*exports*/
	if (json_object_object_get_ex(cfg, "transit", &ary)) {
		add = json_object_array_length(ary);
		p_cfg->exp_count = add;
		assert(p_cfg->exp_count <= MAX_LINK_INDEX);

		for (i = 0; i < add; i++) {
			tmp = json_object_array_get_idx(ary, i);

			if (json_object_object_get_ex(tmp, "port", &obj)) {
				p_cfg->exports[i].port = (short)json_object_get_int(obj);
			} else { goto fail; }

			if (json_object_object_get_ex(tmp, "host", &obj)) {
				str_val = json_object_get_string(obj);
				p_cfg->exports[i].host = x_strdup(str_val);
			} else { goto fail; }
		}
	} else { goto fail; }

	/*inports*/
	if (json_object_object_get_ex(cfg, "weibo", &ary)) {
		add = json_object_array_length(ary);
		p_cfg->inp_count = add;
		assert(p_cfg->inp_count <= MAX_LINK_INDEX);

		for (i = 0; i < add; i++) {
			tmp = json_object_array_get_idx(ary, i);

			if (json_object_object_get_ex(tmp, "port", &obj)) {
				p_cfg->inports[i].port = (short)json_object_get_int(obj);
			} else { goto fail; }

			if (json_object_object_get_ex(tmp, "host", &obj)) {
				str_val = json_object_get_string(obj);
				p_cfg->inports[i].host = x_strdup(str_val);
			} else { goto fail; }
		}
	} else { goto fail; }

	return;

fail:
	fprintf(stderr, "invalid route config file :%s\n", name);
	exit(0);
}

