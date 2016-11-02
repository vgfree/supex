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
}

static void free_rr_cfg(struct rr_cfg_file *p_cfg)
{
	int i = 0;

	assert(p_cfg);

	for (i = 0; i < DIM(p_cfg->links); i++) {
		Free(p_cfg->links[i].host);
	}

	Free(p_cfg->gaode_host);
	Free(p_cfg->tsdb_host);
	Free(p_cfg->sqlconf.host);
	Free(p_cfg->sqlconf.username);
	Free(p_cfg->sqlconf.password);
	Free(p_cfg->sqlconf.database);
	Free(p_cfg->sqlconf.table);
}

int cmp(const void *a, const void *b)
{
	return *(int *)a - *(int *)b;
}

bool read_rr_cfg(struct rr_cfg_file *p_cfg, char *name)
{
	int             i = 0;
	int             idx = 0;
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

	if (!json_object_object_get_ex(cfg, "citycode", &ary)) {
		x_printf(E, "can not find [citycode].");
		goto fail;
	}

	int city_len = json_object_array_length(ary);
	p_cfg->city_size = city_len;

	for (i = 0; i < city_len; i++) {
		struct json_object *tmp = json_object_array_get_idx(ary, i);
		p_cfg->citycode[i] = json_object_get_int(tmp);
	}

	qsort(p_cfg->citycode, city_len, sizeof(p_cfg->citycode[0]), cmp);

	if (!json_object_object_get_ex(cfg, "gaode_host", &obj)) {
		x_printf(E, "can not find [gaode_host]\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->gaode_host = x_strdup(str_val);

	if (!json_object_object_get_ex(cfg, "gaode_port", &obj)) {
		x_printf(E, "can not find [gaode_port]\n");
		goto fail;
	}

	p_cfg->gaode_port = json_object_get_int(obj);

	if (!json_object_object_get_ex(cfg, "synctime", &obj)) {
		x_printf(E, "can not find [synctime]\n");
		goto fail;
	}

	p_cfg->synctime = json_object_get_int(obj);

	if (!json_object_object_get_ex(cfg, "sqlhost", &obj)) {
		x_printf(E, "can not find [sqlhost]\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->sqlconf.host = x_strdup(str_val);

	if (!json_object_object_get_ex(cfg, "sqluser", &obj)) {
		x_printf(E, "can not find [sqluser]\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->sqlconf.username = x_strdup(str_val);

	if (!json_object_object_get_ex(cfg, "sqlpass", &obj)) {
		x_printf(E, "can not find [sqlpass]\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->sqlconf.password = x_strdup(str_val);

	if (!json_object_object_get_ex(cfg, "sqldb", &obj)) {
		x_printf(E, "can not find [sqldb]\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->sqlconf.database = x_strdup(str_val);

	if (!json_object_object_get_ex(cfg, "sqlstr", &obj)) {
		x_printf(E, "can not find [sqlstr]\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->sqlconf.sql = x_strdup(str_val);

	if (!json_object_object_get_ex(cfg, "sqltab", &obj)) {
		x_printf(E, "can not find [sqltab]\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->sqlconf.table = x_strdup(str_val);

	if (!json_object_object_get_ex(cfg, "sqlport", &obj)) {
		x_printf(E, "can not find [sqlport]\n");
		goto fail;
	}

	p_cfg->sqlconf.port = json_object_get_int(obj);

	if (!json_object_object_get_ex(cfg, "tsdb_host", &obj)) {
		x_printf(E, "can not find [tsdb_host]\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->tsdb_host = x_strdup(str_val);

	if (!json_object_object_get_ex(cfg, "tsdb_port", &obj)) {
		x_printf(E, "can not find [tsdb_port]\n");
		goto fail;
	}

	p_cfg->tsdb_port = json_object_get_int(obj);

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

