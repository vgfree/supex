#pragma once
#include "rr_def.h"
#include "single_model.h"
#include "subsection_model_v2.h"
#include "ev.h"

typedef struct single_model_t SINGLE_MODEL; 
typedef struct subsec_model_t SUBSEC_MODEL; 

typedef struct road_grade
{
        unsigned short def;
        unsigned short highway;
        unsigned short lowway;
} ROAD_GRADE;

typedef struct traffic_model_t
{
        ROAD_GRADE   rg;
        SINGLE_MODEL sin;
        SUBSEC_MODEL sub;
} TRAFFIC_MODEL;

void load_traffic_model_cfgfile(TRAFFIC_MODEL *p_cfg, char *name);
void mount_model( TRAFFIC_MODEL *model_cfg );
int calculate_start(struct ev_loop *p_loop, road_info_t *road_info, gps_info_t *gps_info, TRAFFIC_MODEL *model_cfg);
