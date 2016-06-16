#pragma once

#include "map_pmr.h"
#include <stdint.h>
#include "user_info.h"

typedef struct match_road_t
{
	gps_point_t     cur_pt;
	short           match_size;
	road_info_t     roads[MAX_MATCH_ROAD_SIZE];
} match_road_t;

int match_roads(match_road_t *p_match, char *p_imei);

