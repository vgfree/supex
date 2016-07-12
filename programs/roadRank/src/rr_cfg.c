#include <json.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "rr_cfg.h"

static void init_rr_cfg(struct rr_cfg_file *p_cfg)
{
	assert(p_cfg);
	memset(p_cfg, 0, sizeof(*p_cfg));
}

static void copy_rr_cfg(struct rr_cfg_file *dest, struct rr_cfg_file *src)
{
	assert(dest);
	assert(src);

	*dest = *src;
	dest->appKey = x_strdup(src->appKey);
	dest->secret = x_strdup(src->secret);
}

static void free_rr_cfg(struct rr_cfg_file *p_cfg)
{
	assert(p_cfg);

	Free(p_cfg->appKey);
	Free(p_cfg->secret);
}

bool fill_link(struct json_object *obj, char *obj_name, struct rr_link *p_link)
{
	struct json_object      *sub_obj = NULL;
	struct json_object      *host_obj = NULL;
	struct json_object      *port_obj = NULL;

	if (!obj || !obj_name || !p_link) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(obj, obj_name, &sub_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "host", &host_obj)) {
		goto fill_fail;
	}

	if (!json_object_object_get_ex(sub_obj, "port", &port_obj)) {
		goto fill_fail;
	}

	const char      *server_host = json_object_get_string(host_obj);
	short           server_port = (short)json_object_get_int(port_obj);

	memset(p_link->host, 0, sizeof(p_link->host));
	strncpy(p_link->host, server_host, sizeof(p_link->host) - 1);
	p_link->port = server_port;

	return true;

fill_fail:
	x_printf(E, "cann't find %s \n", obj_name);
	return false;
}

bool read_rr_cfg(struct rr_cfg_file *p_cfg, char *name)
{
	const char *str_val = NULL;

	struct json_object      *obj = NULL;
	struct json_object      *cfg = NULL;
	struct json_object      *ary = NULL;
	struct rr_cfg_file      oldcfg = {};

	assert(p_cfg);
	assert(name);

	copy_rr_cfg(&oldcfg, p_cfg);

	init_rr_cfg(p_cfg);

	cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	// appKey
	if (!json_object_object_get_ex(cfg, "appKey", &obj)) {
		x_printf(E, "can't found appKey!\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->appKey = x_strdup(str_val);

	// secret
	if (!json_object_object_get_ex(cfg, "secret", &obj)) {
		x_printf(E, "can't found secret!\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->secret = x_strdup(str_val);

	// save_time
	if (!json_object_object_get_ex(cfg, "save_time", &obj)) {
		x_printf(E, "can't found save_time!\n");
		goto fail;
	}

	p_cfg->save_time = (int)json_object_get_int(obj);

	// kv_cache_count
	if (!json_object_object_get_ex(cfg, "kv_cache_count", &obj)) {
		x_printf(E, "can't found kv_cache_count!\n");
		goto fail;
	}

	p_cfg->kv_cache_count = (int)json_object_get_int(obj);

	// model cfg file name
	if (!json_object_object_get_ex(cfg, "model_cfg_file", &obj)) {
		x_printf(E, "can't found model_cfg_file!\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);
	p_cfg->model_cfg_name = x_strdup(str_val);

	/*
	 *        //model type
	 *        if (!json_object_object_get_ex(cfg, "model_type", &obj)) {
	 *                x_printf(E, "can't found model_type!\n");
	 *                goto fail;
	 *        }
	 *
	 *        str_val = json_object_get_string(obj);
	 *        if( strncmp( str_val, "single", 6) == 0)
	 *                p_cfg->model_type = 0;
	 *        else if( strncmp( str_val, "subsec", 6) == 0)
	 *                p_cfg->model_type = 1;
	 *        else goto fail;
	 */
	// redis_conn
	if (!json_object_object_get_ex(cfg, "redis_conn", &obj)) {
		x_printf(E, "can't found redis_conn!\n");
		goto fail;
	}

	p_cfg->redis_conn = (int)json_object_get_int(obj);

	/*servers*/
	if (!json_object_object_get_ex(cfg, "servers", &obj)) {
		goto fail;
	}

	if (!fill_link(obj, "pmr_server", &(p_cfg->pmr_server))) {
		goto fail;
	}

	if (!fill_link(obj, "trafficapi_server", &(p_cfg->trafficapi_server))) {
		goto fail;
	}

	if (!fill_link(obj, "forward_server", &(p_cfg->forward_server))) {
		goto fail;
	}

	/*links*/
	if (!json_object_object_get_ex(cfg, "links", &obj)) {
		x_printf(E, "can't found [links].");
		goto fail;
	}

	if (!fill_link(obj, "road_traffic", &(p_cfg->road_traffic_server))) {
		goto fail;
	}

	if (!fill_link(obj, "city_traffic", &(p_cfg->city_traffic_server))) {
		goto fail;
	}

	if (!fill_link(obj, "county_traffic", &(p_cfg->county_traffic_server))) {
		goto fail;
	}

	/*forward_imei*/
	if (!json_object_object_get_ex(cfg, "forward_imei", &ary)) {
		x_printf(E, "can't found [forward_imei].");
		goto fail;
	}

	int add = json_object_array_length(ary);
	p_cfg->imei_count = add;

	if (p_cfg->imei_count > MAX_TEST_IMEI) {
		x_printf(E, "the member of [forward_imei] is too much.");
		goto fail;
	}

	int i;

	for (i = 0; i < add; i++) {
		struct json_object *tmp = NULL;
		tmp = json_object_array_get_idx(ary, i);
                strcpy(p_cfg->imei_buff[i],json_object_get_string(tmp));
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

