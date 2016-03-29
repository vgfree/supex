#include <json.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "rr_cfg.h"

static void init_rr_cfg(struct rr_cfg_file *p_cfg)
{
	//        int i = 0;

	assert(p_cfg);

	//        for (i = 0; i < DIM(p_cfg->links); i++) {
	//                Free(p_cfg->links[i].host);
	//        }

	memset(p_cfg, 0, sizeof(*p_cfg));
}

static void copy_rr_cfg(struct rr_cfg_file *dest, struct rr_cfg_file *src)
{
	int i = 0;

	assert(dest);
	assert(src);

	for (i = 0; i < DIM(dest->links); i++) {
		dest->links[i].host = x_strdup(src->links[i].host);
		dest->links[i].port = src->links[i].port;
	}

	dest->appKey = x_strdup(src->appKey);
	dest->secret = x_strdup(src->secret);
	dest->map_server_host = x_strdup(src->map_server_host);
	dest->map_server_port = src->map_server_port;
}

static void free_rr_cfg(struct rr_cfg_file *p_cfg)
{
	int i = 0;

	assert(p_cfg);

	for (i = 0; i < DIM(p_cfg->links); i++) {
		Free(p_cfg->links[i].host);
	}

	Free(p_cfg->appKey);
	Free(p_cfg->secret);
	Free(p_cfg->map_server_host);
}

bool read_rr_cfg(struct rr_cfg_file *p_cfg, char *name)
{
	int             i = 0;
	int             add = 0;
	const char      *str_val = NULL;

	struct json_object      *ary = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = NULL;
	struct rr_cfg_file      oldcfg = {};

	assert(p_cfg);
	assert(name);

	copy_rr_cfg(&oldcfg, p_cfg);

	init_rr_cfg(p_cfg);

	cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	if (!json_object_object_get_ex(cfg, "appKey", &obj)) {
		x_printf(E, "can't found appKey!\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->appKey = x_strdup(str_val);

	if (!json_object_object_get_ex(cfg, "secret", &obj)) {
		x_printf(E, "can't found secret!\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->secret = x_strdup(str_val);

	if (!json_object_object_get_ex(cfg, "map_server_host", &obj)) {
		x_printf(E, "can't found map_server_host!\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->map_server_host = x_strdup(str_val);

	if (!json_object_object_get_ex(cfg, "map_server_port", &obj)) {
		x_printf(E, "can't found map_server_port!\n");
		goto fail;
	}

	p_cfg->map_server_port = (short)json_object_get_int(obj);

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

	free_rr_cfg(&oldcfg);

	json_object_put(cfg);

	return true;

fail:

	if (cfg) {
		json_object_put(cfg);
	}

	free_rr_cfg(p_cfg);

	copy_rr_cfg(p_cfg, &oldcfg);

	free_rr_cfg(&oldcfg);

	x_printf(E, "invalid rr config file :%s", name);

	return false;
}

