#pragma once

#include "rr_def.h"
#include "single_cfg.h"
#include "kv_road_v2.h"
#include "kv_imei.h"
#include "gps_info.h"
#include "road_info.h"
#include "ev.h"

typedef struct single_cfg_file SINGLE_CFG;

// typedef int (*UNLIKE_CALLBACK)(struct ev_loop *loop, KV_IMEI kv_IMEI, KV_ROADID kv_roadID, gps_info_t *gps_info, road_info_t *road_info, struct single_cfg_file cfg);
// typedef int (*LIKE_CALLBACK)(struct gps_info_t *gps_info, struct road_info_t *road_info, KV_IMEI kv_IMEI);
typedef int (*UPDATE_REDIS_CALLBACK)(SECKV_ROAD *kv_roadID, int g_save_time, struct ev_loop *loop);

struct single_model_t
{
	SINGLE_CFG              single_cfg;
	// LIKE_CALLBACK   like_call;
	// UNLIKE_CALLBACK ulike_call;
	UPDATE_REDIS_CALLBACK   single_update;
};

int single_calculate(struct ev_loop *p_loop, gps_info_t *gps_info, road_info_t *road_info, struct single_model_t *single);

int single_new_road_2(struct ev_loop *loop, KV_IMEI kv_IMEI, SECKV_ROAD kv_roadID, gps_info_t *gps_info, road_info_t *road_info, int limit);

