#pragma once

#include "rr_def.h"
#include "subsection_cfg.h"
#include "kv_road.h"
#include "ev.h"
#include "gps_info.h"
#include "road_info.h"

typedef struct subsec_cfg_file SUBSEC_CFG;

typedef int (*UPDATE_REDIS_CALLBACK)( SECKV_ROAD *kv_roadID, int g_save_time, struct ev_loop *loop );
typedef void (*UPDATE_ROADSEC_CALLBACK)( SECKV_ROAD *kv_road, gps_info_t *gps_info, road_info_t *road_info, int merged_speed_limit, int replace_limit, int expire_time );
typedef void (*INIT_ROADSEC_CALLBACK)(gps_info_t *gps_info, road_info_t *road_info, SECKV_ROAD *kv_road);

struct subsec_model_t
{
        SUBSEC_CFG subsec_cfg;
        UPDATE_REDIS_CALLBACK section_update;
        UPDATE_ROADSEC_CALLBACK update_roadsec;
        INIT_ROADSEC_CALLBACK init_roadsec;
};

int subsec_calculate( struct ev_loop * p_loop, gps_info_t *gps_info, road_info_t *road_info, struct subsec_model_t *subsec);
void init_SECROAD_data(gps_info_t *gps_info, road_info_t *road_info, SECKV_ROAD *kv_road);
void init_SECROAD_data2(gps_info_t *gps_info, road_info_t *road_info, SECKV_ROAD *kv_road);
void update_roadsection( SECKV_ROAD *kv_road, gps_info_t *gps_info, road_info_t *road_info, int merged_speed_limit, int replace_limit, int expire_time );
void set_CURRENTROAD_data( SECKV_ROAD *kv_road, gps_info_t *gps_info, road_info_t *road_info, int merged_speed_limit, int replace_limit, int expire_time );
