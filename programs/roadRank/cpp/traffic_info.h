#pragma once
#include "road_info.h"
#include "rr_def.h"
#include "kv_road_v2.h"
#include "ev.h"

typedef struct traffic_info_t
{} traffic_info_t;

int single_update_redis(SECKV_ROAD *kv_roadID, int g_save_time, struct ev_loop *loop);

int subsec_update_redis(SECKV_ROAD *kv_roadID, int g_save_time, struct ev_loop *loop);

