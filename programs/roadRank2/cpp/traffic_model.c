#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "traffic_info.h"
#include "traffic_model.h"
#include "json.h"
#include "road_info.h"

void load_traffic_model_cfgfile(TRAFFIC_MODEL *p_cfg, char *name)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	if (!json_object_object_get_ex(cfg, "default", &obj)) {
		x_printf(E, "can't found model_type!\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);

	if (strncmp(str_val, "single", 6) == 0) {
		p_cfg->rg.def = 0;
	} else if (strncmp(str_val, "subsec", 6) == 0) {
		p_cfg->rg.def = 1;
	} else {
		goto fail;
	}

	if (!json_object_object_get_ex(cfg, "highway", &obj)) {
		x_printf(E, "can't found model_type!\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);

	if (strncmp(str_val, "single", 6) == 0) {
		p_cfg->rg.highway = 0;
	} else if (strncmp(str_val, "subsec", 6) == 0) {
		p_cfg->rg.highway = 1;
	} else {
		goto fail;
	}

	if (!json_object_object_get_ex(cfg, "lowway", &obj)) {
		x_printf(E, "can't found model_type!\n");
		goto fail;
	}

	str_val = json_object_get_string(obj);

	if (strncmp(str_val, "single", 6) == 0) {
		p_cfg->rg.lowway = 0;
	} else if (strncmp(str_val, "subsec", 6) == 0) {
		p_cfg->rg.lowway = 1;
	} else {
		goto fail;
	}

	if (!json_object_object_get_ex(cfg, "model", &obj)) {
		goto fail;
	}

	if (!fill_single_model(obj, "single_mod", &(p_cfg->sin.single_cfg))) {
		goto fail;
	}

	if (!fill_subsec_model(obj, "subsec_mod", &(p_cfg->sub.subsec_cfg))) {
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

void mount_model(TRAFFIC_MODEL *model_cfg)
{
	model_cfg->sin.single_update = single_update_redis;
	model_cfg->sub.section_update = subsec_update_redis;
	model_cfg->sub.update_roadsec = update_roadsection;
	// model_cfg->sub.init_roadsec     = init_SECROAD_data;
	// model_cfg->sub.update_roadsec   = set_CURRENTROAD_data;
	model_cfg->sub.init_roadsec = init_SECROAD_data2;
}

int calculate_start(struct ev_loop *p_loop, road_info_t *road_info, gps_info_t *gps_info, TRAFFIC_MODEL *model_cfg)
{
	if ((road_info->rt == HIGHWAY) || (road_info->rt == EXPRESSWAY)) {
		if (model_cfg->rg.highway == SINGLE) {
			single_calculate(p_loop, gps_info, road_info, &model_cfg->sin);
		} else {
			subsec_calculate(p_loop, gps_info, road_info, &model_cfg->sub);
		}
	} else {
		if (model_cfg->rg.lowway == SINGLE) {
			single_calculate(p_loop, gps_info, road_info, &model_cfg->sin);
		} else {
			subsec_calculate(p_loop, gps_info, road_info, &model_cfg->sub);
		}
	}

	return 1;
}

