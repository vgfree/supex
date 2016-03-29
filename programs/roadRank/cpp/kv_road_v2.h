#ifndef __KV_ROAD_H_
#define __KV_ROAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rr_def.h"
#include "utils.h"
#include "libkv.h"
#include "libmini.h"

typedef struct road_section_t {
        unsigned short begin;
        unsigned short end;
        unsigned short avg_speed;
        unsigned short max_speed;
        unsigned short used_time;
        long    endtime;
        double longitude;                       /*gps最新点经度*/
        double latitude;                        /*gps最新点纬度*/
} SITE;

typedef struct seckv_road
{
        char            shift;
        SITE            road_sec[ROADSECNUM]; //FIXME maybe not enough
        AO_SpinLockT            locker;;
        int             sec_num;
        int             max_speed;
        int             avg_speed;
        int             citycode;
        int             countycode;
        long long       IMEI;
        long            end_time;
        long            used_time;
        long            old_roadID;
} SECKV_ROAD;

void show_road_section( char *ret, SECKV_ROAD *road );
void single_road_section( char *ret, SECKV_ROAD *road );
int roadsec_info_get( long roadid, SECKV_ROAD **roadsec);
int roadsec_info_save( SECKV_ROAD *kv_road );
int set_roadID_to_kv( SECKV_ROAD *kv_roadID );
#endif	/* ifndef __KV_ROAD_V2_H_ */

