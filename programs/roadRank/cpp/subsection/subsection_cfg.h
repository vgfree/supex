#pragma once

#include "json.h"
#include "utils.h"

struct subsec_cfg_file 
{
        unsigned short road_match_limit;
        unsigned short replace_limit_l;
        unsigned short replace_limit_h;
        unsigned short kv_cache_count;
        unsigned short expire_time;
        unsigned short merged_speed_l; 
        unsigned short merged_speed_h; 
        unsigned short init_max; 

};

bool fill_subsec_model(struct json_object *obj, char *obj_name, struct subsec_cfg_file *p_link);
