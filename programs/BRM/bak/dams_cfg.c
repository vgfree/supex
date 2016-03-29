#include <json.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "dams_cfg.h"

static void init_dams_cfg(struct dams_cfg_file *p_cfg)
{
	//        int i = 0;

	assert(p_cfg);

	//        for (i = 0; i < DIM(p_cfg->links); i++) {
	//                Free(p_cfg->links[i].host);
	//        }

	memset(p_cfg, 0, sizeof(*p_cfg));
	memset(p_cfg->fresh, NO_SET_UP, DIM(p_cfg->fresh));
	memset(p_cfg->delay, NO_SET_UP, DIM(p_cfg->delay));
}

static void copy_dams_cfg(struct dams_cfg_file *dest, struct dams_cfg_file *src)
{
	int i = 0;

	assert(dest);
	assert(src);

	for (i = 0; i < DIM(dest->links); i++) {
		dest->links[i].host = x_strdup(src->links[i].host);
		dest->links[i].port = src->links[i].port;
	}

	memcpy(dest->fresh, src->fresh, sizeof(src->fresh));
	memcpy(dest->delay, src->delay, sizeof(src->delay));
}

static void free_dams_cfg(struct dams_cfg_file *p_cfg)
{
	int i = 0;

	assert(p_cfg);

	for (i = 0; i < DIM(p_cfg->links); i++) {
		Free(p_cfg->links[i].host);
	}
}

bool read_dams_cfg(struct dams_cfg_file *p_cfg, char *name)
{
	int             i = 0;
	int             idx = 0;
	int             add = 0;
	const char      *str_val = NULL;

	struct json_object      *ary = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = NULL;
	struct dams_cfg_file    oldcfg = {};

	assert(p_cfg);
	assert(name);

	copy_dams_cfg(&oldcfg, p_cfg);

	init_dams_cfg(p_cfg);

	cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	/*links*/
	if (!json_object_object_get_ex(cfg, "links", &ary)) {
		x_printf(E, "can't found [links].");
		goto fail;
	}

	add = json_object_array_length(ary);
	p_cfg->count = add;

	if (p_cfg->count > DIM(p_cfg->links)) {
		x_printf(E, "the member of [links] is too much.");
		goto fail;
	}

	for (i = 0; i < add; i++) {
		struct json_object *tmp = NULL;

		tmp = json_object_array_get_idx(ary, i);

		if (!json_object_object_get_ex(tmp, "port", &obj)) {
			x_printf(E, "can't found [port] that is member of [links]. ");
			goto fail;
		}

		p_cfg->links[i].port = (short)json_object_get_int(obj);

		if (!json_object_object_get_ex(tmp, "host", &obj)) {
			x_printf(E, "can't found [host] that is member of [links]. ");
			goto fail;
		}

		str_val = json_object_get_string(obj);
		p_cfg->links[i].host = x_strdup(str_val);
	}

	/*fresh*/
	if (!json_object_object_get_ex(cfg, "fresh", &ary)) {
		x_printf(E, "can't found [fresh].");
		goto fail;
	}

	add = json_object_array_length(ary);

	if ((add > p_cfg->count) || (add > DIM(p_cfg->fresh))) {
		x_printf(E, "the member of [fresh] is too much.");
		goto fail;
	}

	for (i = 0; i < add; i++) {
		obj = json_object_array_get_idx(ary, i);
		idx = (short)json_object_get_int(obj);

		if ((idx > p_cfg->count) || (idx > DIM(p_cfg->fresh))) {
			x_printf(E, "the value of index that indicate which [links] in [fresh]. ");
			goto fail;
		}

		p_cfg->fresh[idx] = IS_SET_UP;
	}

	/*delay*/
	if (!json_object_object_get_ex(cfg, "delay", &ary)) {
		x_printf(E, "can't found [delay].");
		goto fail;
	}

	add = json_object_array_length(ary);

	if ((add > p_cfg->count) || (add > DIM(p_cfg->delay))) {
		x_printf(E, "the member of [delay] is too much.");
		goto fail;
	}

	for (i = 0; i < add; i++) {
		obj = json_object_array_get_idx(ary, i);
		idx = (short)json_object_get_int(obj);

		if ((idx > p_cfg->count) || (idx > DIM(p_cfg->delay))) {
			x_printf(E, "the value of index that indicate which [links] in [delay]. ");
			goto fail;
		}

		p_cfg->delay[idx] = IS_SET_UP;
	}

	free_dams_cfg(&oldcfg);

	json_object_put(cfg);

	return true;

fail:

	if (cfg) {
		json_object_put(cfg);
	}

	free_dams_cfg(p_cfg);

	copy_dams_cfg(p_cfg, &oldcfg);

	free_dams_cfg(&oldcfg);

	x_printf(E, "invalid dams config file :%s", name);

	return false;
}

