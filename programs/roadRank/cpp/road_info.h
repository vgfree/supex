#pragma once
#include "ev.h"
#include "rr_cfg.h"
#include "gps_info.h"

typedef struct road_info_t {
        unsigned short  sg_id;
        unsigned short  rt;
        unsigned int    rr_id;
        long            new_roadID;
        unsigned int    county_code;
        unsigned int    city_code;
        unsigned int    len;
        double          start_lon;
        double          start_lat;
        double          end_lon;
        double          end_lat;
        char            road_name[100];
} road_info_t;

typedef struct pmr_info_t {
        struct ev_loop *p_loop;
        rr_link pmr_link;
        gps_info_t p_gps;
        void (*pmr_callback)(struct ev_loop *, gps_info_t *, road_info_t *);
} pmr_info_t;

int get_road_info(pmr_info_t *p_pmr);
