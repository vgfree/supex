#pragma once
#include "map_seg.h"

#define MAX_MATCH_ROAD_SIZE 30
#define IMEI_LEN 15
#define MLOCATE_ROAD_DEEP 3

typedef struct gps_point_t {
        long long time;
        double lon;
        double lat;
        short dir;
        unsigned short speed;
        short alt;
}gps_point_t;

typedef struct road_info_t {
        map_seg_info *p_sg;
        char *p_sg_name;
        gps_point_t start_point;
        gps_point_t end_point;
        short dist;
        short sub_dir;
        double weight;
        long status;
} road_info_t;

typedef struct user_info_t {
        char IMEI[IMEI_LEN + 1];
        road_info_t roads[MLOCATE_ROAD_DEEP];
        int count;
} user_info_t;

user_info_t* user_info_get(char *p_imei);

int user_info_save(user_info_t *p_user);

int user_info_add(user_info_t *p_user, road_info_t *p_road);
