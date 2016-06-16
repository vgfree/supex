#pragma once
#include "json.h"
#include "utils.h"

struct single_cfg_file
{
	unsigned short  road_match_limit;
	unsigned short  kv_cache_count;
	unsigned short  expire_time;
};

bool fill_single_model(struct json_object *obj, char *obj_name, struct single_cfg_file *p_link);

