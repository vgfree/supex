#include "subsection_model.h"
#include "async_tasks/async_api.h"
#include "rr_def.h"
#include "rr_cfg.h"
#include "gps_info.h"
#include "road_info.h"
#include "single_model.h"
#include "utils.h"

#include <math.h>
#include <time.h> 

extern struct rr_cfg_file g_rr_cfg_file;

static unsigned int dist_p2p(double from_lon, double from_lat, double to_lon, double to_lat)
{
        if ((fabs(from_lon - to_lon) <= 0.00001) && (fabs(from_lat - to_lat) <= 0.00001)) {
                return 0;
        }

        double distance = 0.0;
        distance = EARTH_RADIUS * acos(sin(from_lat / 57.2958) * sin(to_lat / 57.2958) + cos(from_lat / 57.2958) * cos(to_lat / 57.2958) * cos((from_lon - to_lon) / 57.2958)) * 1000;
        return floor(distance);
}

static int get_begin_lenth(double from_lon, double from_lat, double b_lon, double b_lat, double e_lon, double e_lat, int a)
{
        unsigned int b = dist_p2p(from_lon, from_lat, b_lon, b_lat);
        unsigned int c = dist_p2p(from_lon, from_lat, e_lon, e_lat);
        if (0 == b && 0 == a ) {
                x_printf(E, "get begin lenth failed..\n");
                return 0;
        }

        return ( (a*a + b*b - c*c) / (2*a) );
}

static int get_end_lenth( int B, unsigned short avg_speed, long t)
{
        return ( avg_speed * 1000 * t / (60 * 60)  + B);
}

static void set_end_roadsection( SITE *sec, unsigned short avg, long endtime )
{
        sec->endtime = endtime;
        sec->avg_speed = avg;
}

static void set_roadsection(SITE *sec, unsigned short begin, unsigned short end, unsigned short avg, long endtime, double lon, double lat)
{
        sec->begin = begin;
        if( end )
                sec->end   = end;
        sec->avg_speed = avg;
        if( endtime )
                sec->endtime = endtime;
        if( lon )
                sec->longitude = lon;
        if( lat )
                sec->latitude = lat;
}

static void delete_roadsection(SITE *sec, int B, int E, int num)
{
        if( B > E )
                return;
        int i;
        for( i = E + 1; i < num; i++ ) {
                set_roadsection( &sec[B], sec[i].begin, sec[i].end, sec[i].avg_speed, sec[i].endtime, sec[i].longitude, sec[i].latitude );
                B += 1;
        }
}

static void expend_roadsection(SITE *sec, int B, int num)
{
        if( B > num ) {
                x_printf(E, "expend sec error B >= num\n");
                return;
        }
        int i;
        for( i = num; i >= B; i-- ) {
                set_roadsection( &sec[i], sec[i-1].begin, sec[i-1].end, sec[i-1].avg_speed, sec[i-1].endtime, sec[i-1].longitude, sec[i-1].latitude );
        }
}

void init_SECROAD_data(gps_info_t *gps_info, road_info_t *road_info, SECKV_ROAD *kv_road)
{
        kv_road->IMEI = atoll(gps_info->IMEI);
        kv_road->old_roadID = road_info->new_roadID;
        kv_road->end_time = gps_info->end_time;
        kv_road->avg_speed = gps_info->avg_speed;
        kv_road->max_speed = gps_info->max_speed;
        kv_road->used_time = gps_info->end_time - gps_info->start_time;
        kv_road->citycode = road_info->city_code;
        kv_road->countycode = road_info->county_code; 
        kv_road->sec_num = 1;
        //FIXME
        kv_road->road_sec[0].begin = get_begin_lenth(gps_info->longitude, gps_info->latitude, road_info->start_lon, road_info->start_lat, road_info->end_lon, road_info->end_lat, road_info->len);
        kv_road->road_sec[0].end   = get_end_lenth(kv_road->road_sec[0].begin, gps_info->avg_speed, gps_info->end_time - gps_info->start_time);
        kv_road->road_sec[0].avg_speed = gps_info->avg_speed;
        kv_road->road_sec[0].endtime = gps_info->end_time;
        kv_road->road_sec[0].longitude = gps_info->longitude;
        kv_road->road_sec[0].latitude = gps_info->latitude;
        x_printf(D, "init highway road sec B:%d E:%d A:%d\n", kv_road->road_sec[0].begin, kv_road->road_sec[0].end, kv_road->road_sec[0].avg_speed);
}

void init_SECROAD_data2(gps_info_t *gps_info, road_info_t *road_info, SECKV_ROAD *kv_road)
{
        kv_road->IMEI = atoll(gps_info->IMEI);
        kv_road->old_roadID = road_info->new_roadID;
        kv_road->end_time = gps_info->end_time;
        kv_road->avg_speed = gps_info->avg_speed;
        kv_road->max_speed = gps_info->max_speed;
        kv_road->used_time = gps_info->end_time - gps_info->start_time;
        kv_road->citycode = road_info->city_code;
        kv_road->countycode = road_info->county_code; 
        kv_road->sec_num = 1;
        //FIXME
        kv_road->road_sec[0].end = get_begin_lenth(gps_info->longitude, gps_info->latitude, road_info->start_lon, road_info->start_lat, road_info->end_lon, road_info->end_lat, road_info->len);
        kv_road->road_sec[0].begin = 0;
        kv_road->road_sec[0].avg_speed = gps_info->avg_speed;
        kv_road->road_sec[0].endtime = gps_info->end_time;
        kv_road->road_sec[0].longitude = gps_info->longitude;
        kv_road->road_sec[0].latitude = gps_info->latitude;

        //设置起点
        kv_road->road_sec[98].end = 0;
        kv_road->road_sec[98].begin = 0;
        //kv_road->road_sec[98].avg_speed = gps_info->avg_speed;
        //kv_road->road_sec[98].endtime = gps_info->end_time;
        kv_road->road_sec[98].longitude = road_info->start_lon;
        kv_road->road_sec[98].latitude =  road_info->start_lat;
        //设置终点
        kv_road->road_sec[99].end = road_info->len;
        kv_road->road_sec[99].begin = 0;
        kv_road->road_sec[99].avg_speed = gps_info->avg_speed;
        kv_road->road_sec[99].endtime = gps_info->end_time;
        kv_road->road_sec[99].longitude = road_info->end_lon;
        kv_road->road_sec[99].latitude  = road_info->end_lat;
        x_printf(D, "init highway road sec B:%d E:%d A:%d\n", kv_road->road_sec[0].begin, kv_road->road_sec[0].end, kv_road->road_sec[0].avg_speed);
}

static void set_road_data(gps_info_t *gps_info, road_info_t *road_info, SECKV_ROAD *kv_road )
{
        kv_road->IMEI = atoll(gps_info->IMEI);
        kv_road->end_time = gps_info->end_time;
        kv_road->avg_speed = (gps_info->avg_speed + kv_road->avg_speed) / 2;
        kv_road->max_speed = MAX( gps_info->max_speed, kv_road->max_speed);
        kv_road->used_time = gps_info->end_time - gps_info->start_time + kv_road->used_time;
        kv_road->old_roadID = road_info->new_roadID;
}

static int get_interval( SECKV_ROAD *kv_road, int B, int x )
{
        int i;
        for( i = B; i < kv_road->sec_num; i++ ) {
                if( kv_road->road_sec[i].end >= x )
                        return i;
        }

        return i;
}

static int comparing_d(int a, int b)
{
        if(a < 0 || b < 0) {
                x_printf(E, "compare d error !!\n");
                return -1;
        }

        return (a < b) ? 1 : 0;
}

static int comparing_t(long a, int b)
{
        if(a < 0 || b < 0) {
                x_printf(E, "compare t error !!\n");
                return -1;
        }

        if(b == 0)
                return 1;

        time_t now_time;       
        time(&now_time);
        int interval = now_time - a;
        x_printf(D, "now - end: %d", interval);
        return (interval < b) ? 1 : 0;
}

void update_roadsection( SECKV_ROAD *kv_road, gps_info_t *gps_info, road_info_t *road_info, int merged_speed_limit, int replace_limit, int expire_time )
{
        int B = get_begin_lenth(gps_info->longitude, gps_info->latitude, road_info->start_lon, road_info->start_lat, road_info->end_lon, road_info->end_lat, road_info->len);
        unsigned short avg_speed = gps_info->avg_speed;
        set_road_data(gps_info, road_info, kv_road);
        SITE *road_sec = kv_road->road_sec;
        long et = gps_info->end_time;
        int num     = kv_road->sec_num;
        int retnumB = get_interval(kv_road, 0, B);

        if(num == 0) {
                kv_road->sec_num = 1;
                set_roadsection(&kv_road->road_sec[0], 0, B, avg_speed, et, gps_info->longitude, gps_info->latitude);
                set_end_roadsection(&kv_road->road_sec[99], avg_speed, et);
                x_printf(E, "section num = 0\n");
                return;
        }

        if( num >= ROADSECNUM) {  
                x_printf(E, "road sec num is over ! num:%d rr_id:%d sg_id:%d", num, road_info->rr_id, road_info->sg_id);
                return;       
        } 

        if( retnumB == num ) {
                int Distance = dist_p2p(road_sec[retnumB-1].longitude, road_sec[retnumB-1].latitude, gps_info->longitude, gps_info->latitude);
                //if( comparing_t(road_sec[retnumB - 1].endtime, expire_time) && comparing_d(abs(road_sec[retnumB - 1].avg_speed - avg_speed), merged_speed_limit) && comparing_d( abs(road_sec[retnumB-1].end - B), replace_limit ) ) {
                if( comparing_t(road_sec[retnumB - 1].endtime, expire_time) && comparing_d(abs(road_sec[retnumB - 1].avg_speed - avg_speed), merged_speed_limit) && comparing_d( Distance, replace_limit ) ) {
                        if( num > 1 ) {
                                avg_speed = (kv_road->road_sec[retnumB - 1].avg_speed + avg_speed) / 2;
                                set_roadsection(&kv_road->road_sec[retnumB-1], 0, B, avg_speed, et, gps_info->longitude, gps_info->latitude);
                                set_end_roadsection(&kv_road->road_sec[99], avg_speed, et);
                                x_printf(D, "merged road sec B:%d A:%d\n", B, avg_speed);
                        }
                        else {
                                kv_road->sec_num += 1;
                                avg_speed = (kv_road->road_sec[retnumB - 1].avg_speed + avg_speed) / 2;
                                set_roadsection(&kv_road->road_sec[retnumB], 0, B, avg_speed, et, gps_info->longitude, gps_info->latitude);
                                set_end_roadsection(&kv_road->road_sec[99], avg_speed, et);
                                x_printf(D, "merged road sec B:%d A:%d\n", B, avg_speed);
                        
                        }
                }
                else {
                        kv_road->sec_num += 1;
                        set_roadsection(&kv_road->road_sec[retnumB], 0, B, avg_speed, et, gps_info->longitude, gps_info->latitude);
                        set_end_roadsection(&kv_road->road_sec[99], avg_speed, et);
                        x_printf(D, "add road sec B:%d A:%d\n", B, avg_speed);
                }
        }
        else {
                if( comparing_t(road_sec[retnumB].endtime, expire_time) == 0 ) {
                        set_roadsection(&kv_road->road_sec[retnumB], 0, B, gps_info->avg_speed, et, gps_info->longitude, gps_info->latitude);
                        x_printf(D, "expire replace road sec B:%d A:%d\n", B, avg_speed);
                        return;
                }

                int Distance = dist_p2p(road_sec[retnumB].longitude, road_sec[retnumB].latitude, gps_info->longitude, gps_info->latitude);
                //if( abs(kv_road->road_sec[retnumB].end - B) < replace_limit ) {
                if( comparing_d( Distance, replace_limit ) ) {
                        set_roadsection(&kv_road->road_sec[retnumB], 0, B, gps_info->avg_speed, et, gps_info->longitude, gps_info->latitude);
                        x_printf(D, "replace road sec B:%d A:%d distance:%d\n", B, avg_speed, Distance);
                }
                else {
                        expend_roadsection(kv_road->road_sec, retnumB + 1, num);
                        kv_road->sec_num += 1;
                        set_roadsection(&kv_road->road_sec[retnumB], 0, B, avg_speed, et, gps_info->longitude, gps_info->latitude);
                        x_printf(D, "split road sec B:%d A:%d distabce:%d\n",B, avg_speed, Distance);
                }
        }

}

void set_CURRENTROAD_data( SECKV_ROAD *kv_road, gps_info_t *gps_info, road_info_t *road_info, int merged_speed_limit, int replace_limit, int expire_time )
{
        int B = get_begin_lenth(gps_info->longitude, gps_info->latitude, road_info->start_lon, road_info->start_lat, road_info->end_lon, road_info->end_lat, road_info->len);
        int E = get_end_lenth(B, gps_info->avg_speed, gps_info->end_time - gps_info->start_time);
        unsigned short avg_speed = gps_info->avg_speed;
        set_road_data(gps_info, road_info, kv_road);
        SITE *road_sec = kv_road->road_sec;
        long et = gps_info->end_time;

        if(kv_road->sec_num == 0) {
                kv_road->sec_num = 1;
                set_roadsection(&kv_road->road_sec[0], B, E, avg_speed, et, gps_info->longitude, gps_info->latitude);
                x_printf(E, "section num = 0\n");
                return;
        }

        int num     = kv_road->sec_num;
        int retnumB = get_interval(kv_road, 0, B);
        int retnumE = get_interval(kv_road, retnumB, E);
       
        if( num >= ROADSECNUM) {
                x_printf(E, "road sec num is over ! num:%d rr_id:%d sg_id:%d", num, road_info->rr_id, road_info->sg_id);
                return;
        }
        /*超越当前最大段 合并重置*/
        if( retnumB == num ) {
                if( comparing_t(road_sec[retnumB - 1].endtime, expire_time) && comparing_d(abs(road_sec[retnumB - 1].avg_speed - avg_speed), merged_speed_limit) && comparing_d(abs(road_sec[retnumB-1].end - B), replace_limit) ) {
                        avg_speed = (kv_road->road_sec[retnumB - 1].avg_speed + avg_speed) / 2;
                        set_roadsection(&kv_road->road_sec[retnumB-1], kv_road->road_sec[retnumB-1].begin, E, avg_speed, et, kv_road->road_sec[retnumB-1].longitude, kv_road->road_sec[retnumB-1].latitude);
                        x_printf(D, "merged road sec B:%d E:%d A:%d\n", kv_road->road_sec[retnumB-1].begin, E, avg_speed);
                }
                else {
                        kv_road->sec_num += 1;
                        set_roadsection(&kv_road->road_sec[retnumB], B, E, gps_info->avg_speed, et, gps_info->longitude, gps_info->latitude);
                        x_printf(D, "add road sec B:%d E:%d A:%d\n", B, E, avg_speed);
                }
                return;
        }

        int setnum = retnumB - retnumE;

        

        if( retnumE == num ) {
                if( comparing_t(road_sec[retnumB].endtime, expire_time) == 0 ) { 
                        set_roadsection(&kv_road->road_sec[retnumB], B, E, avg_speed, et, gps_info->longitude, gps_info->latitude);
                        kv_road->sec_num -= (setnum - 1);
                        x_printf(D, "expire replace road sec B:%d E:%d A:%d\n", B, E, gps_info->avg_speed);
                        return;
                }

                if( abs(kv_road->road_sec[retnumB].begin - B) < replace_limit) {
                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, E, avg_speed, et, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                        kv_road->sec_num -= (setnum - 1);
                        x_printf(D, "replace road sec B:%d E:%d A:%d\n", kv_road->road_sec[retnumB].begin, E, gps_info->avg_speed);
                }
                else {
                        set_roadsection(&kv_road->road_sec[retnumB + 1], B, E, avg_speed, et, gps_info->longitude, gps_info->latitude);
                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, B, kv_road->road_sec[retnumB].avg_speed, 0, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                        kv_road->sec_num += 1;
                        x_printf(D, "replace road sec B1:%d E1:%d A1:%d\n", kv_road->road_sec[retnumB].begin, B, kv_road->road_sec[retnumB].avg_speed);
                        x_printf(D, "replace road sec B2:%d E2:%d A2:%d\n", B, E, avg_speed);
                }
        }
        else {
        /*最新数据在当前段区间内 替代当前区间 后期可考虑合并左右区间*/
                if( comparing_t(road_sec[retnumB].endtime, expire_time) == 0 ) { 
                        set_roadsection(&kv_road->road_sec[retnumB], B, E, avg_speed, et, gps_info->longitude, gps_info->latitude);
                        kv_road->sec_num -= setnum;
                        delete_roadsection(kv_road->road_sec, retnumB + 1, retnumE, kv_road->sec_num);
                        x_printf(D, "expire replace road sec B:%d E:%d A:%d\n", B, E, avg_speed);
                        return;
                }

                if(abs(kv_road->road_sec[retnumB].begin - B) < replace_limit && abs(kv_road->road_sec[retnumE].end - E) < replace_limit) {
                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, kv_road->road_sec[retnumE].end, avg_speed, et, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                        kv_road->sec_num -= setnum;
                        delete_roadsection(kv_road->road_sec, retnumB + 1, retnumE, kv_road->sec_num);
                        x_printf(D, "replace road sec B:%d E:%d A:%d\n", kv_road->road_sec[retnumB].begin, kv_road->road_sec[retnumE].end, avg_speed);
                }
                else if(abs(kv_road->road_sec[retnumB].begin - B) < replace_limit && abs(kv_road->road_sec[retnumE].end - E) > replace_limit) {
                        switch (setnum) {
                                case 0:
                                        expend_roadsection(kv_road->road_sec, retnumB + 1, kv_road->sec_num);
                                        set_roadsection(&kv_road->road_sec[retnumB+1], E, kv_road->road_sec[retnumE].end, kv_road->road_sec[retnumE].avg_speed, kv_road->road_sec[retnumE].endtime, gps_info->longitude, gps_info->latitude);
                                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, E, avg_speed, et, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                                        kv_road->sec_num += 1;
                                        break;
                                case 1:
                                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, E, avg_speed, et, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                                        //set_roadsection(&kv_road->road_sec[retnumB+1], E, kv_road->road_sec[retnumE].end, kv_road->road_sec[retnumE].avg_speed, gps_info->longitude, gps_info->latitude);
                                        set_roadsection(&kv_road->road_sec[retnumB+1], E, kv_road->road_sec[retnumE].end, kv_road->road_sec[retnumE].avg_speed, 0, kv_road->road_sec[retnumB+1].longitude, kv_road->road_sec[retnumB+1].latitude);
                                        break;
                                default:
                                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, E, avg_speed, et, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                                        //set_roadsection(&kv_road->road_sec[retnumB+1], E, kv_road->road_sec[retnumE].end, kv_road->road_sec[retnumE].avg_speed, gps_info->longitude, gps_info->latitude);
                                        set_roadsection(&kv_road->road_sec[retnumB+1], E, kv_road->road_sec[retnumE].end, kv_road->road_sec[retnumE].avg_speed, 0, kv_road->road_sec[retnumE].longitude, kv_road->road_sec[retnumE].latitude);
                                        delete_roadsection(kv_road->road_sec, retnumB + 2, retnumE, kv_road->sec_num);
                                        kv_road->sec_num -= (setnum - 1);
                                        break;
                        }
                }
                else if(abs(kv_road->road_sec[retnumB].begin - B) > replace_limit && abs(kv_road->road_sec[retnumE].end - E) < replace_limit) {
                        switch (setnum) {
                                case 0:
                                        expend_roadsection(kv_road->road_sec, retnumB + 1, kv_road->sec_num);
                                        set_roadsection(&kv_road->road_sec[retnumB+1], B, kv_road->road_sec[retnumE].end, avg_speed, et, gps_info->longitude, gps_info->latitude);
                                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, B, kv_road->road_sec[retnumB].avg_speed, 0, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                                        kv_road->sec_num += 1;
                                        break;
                                case 1:
                                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, B, kv_road->road_sec[retnumB].avg_speed, 0, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                                        set_roadsection(&kv_road->road_sec[retnumB+1], B, kv_road->road_sec[retnumE].end, avg_speed, et, gps_info->longitude, gps_info->latitude);
                                        break;
                                default:
                                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, B, kv_road->road_sec[retnumB].avg_speed, 0, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                                        set_roadsection(&kv_road->road_sec[retnumB+1], B, kv_road->road_sec[retnumE].end, avg_speed, et, gps_info->longitude, gps_info->latitude);
                                        delete_roadsection(kv_road->road_sec, retnumB + 2, retnumE, kv_road->sec_num);
                                        kv_road->sec_num -= (setnum - 1);
                                        break;
                        }
                }
                else {
                        switch (setnum) {
                                case 0:
                                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, kv_road->road_sec[retnumE].end, avg_speed, et, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                                        break;
                                case 1:
                                        expend_roadsection(kv_road->road_sec, retnumB + 2, kv_road->sec_num);
                                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, B, kv_road->road_sec[retnumB].avg_speed, 0, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                                        set_roadsection(&kv_road->road_sec[retnumB+1], B, E, avg_speed, et, gps_info->longitude, gps_info->latitude);
                                        kv_road->sec_num += 1;
                                        break;
                                default:
                                        set_roadsection(&kv_road->road_sec[retnumB], kv_road->road_sec[retnumB].begin, B, kv_road->road_sec[retnumB].avg_speed, 0, kv_road->road_sec[retnumB].longitude, kv_road->road_sec[retnumB].latitude);
                                        set_roadsection(&kv_road->road_sec[retnumB+1], B, E, avg_speed, et, gps_info->longitude, gps_info->latitude);
                                        set_roadsection(&kv_road->road_sec[retnumB+2], E, kv_road->road_sec[retnumE].end, kv_road->road_sec[retnumE].avg_speed, 0, kv_road->road_sec[retnumE].longitude, kv_road->road_sec[retnumE].latitude);
                                        delete_roadsection(kv_road->road_sec, retnumB + 3, retnumE, kv_road->sec_num);
                                        kv_road->sec_num -= (setnum - 2);
                                        break;
                        }
                }
        }
}

/*
int sec_new_road(struct ev_loop *loop, KV_IMEI kv_IMEI, SECKV_ROAD kv_road, gps_info_t *gps_info, road_info_t *road_info, int limit) 
{
        if( kv_IMEI.rt == LOWWAY ) {//低速换高速
                return single_new_road(loop, kv_IMEI, kv_road, gps_info, road_info, limit);
        }
#if 0
        //高速换高速 更新上一条道路 产生上一条道路的整段路况
        if (kv_IMEI.count >= limit) {
                set_SWITCHROAD_data(&kv_IMEI, &kv_road);

                if (ERR_ROAD == set_secroad_to_kv(&kv_road)) {
                        x_printf(D, "***set_roadID_to_kv failed***\n");
                        return ERR_IMEI;
                }

                sec_update_redis(&kv_road, loop);
        }
#endif
        //更新当前高速分段路况 不考虑之前的用户产生的路况 直接替代，如果考虑需先查询当前道路分段信息
        memset(&kv_road, 0, sizeof(SECKV_ROAD));
        init_SECROAD_data(gps_info, road_info, &kv_road);
        x_printf(D, "set kv_road.old_roadID %ld\n", kv_road.old_roadID);

        if (ERR_ROAD == set_secroad_to_kv(&kv_road)) {
                x_printf(D, "***set_IMEI_to_kv failed***\n");
                return ERR_ROAD;
        }

        sec_update_redis();
#if 0
        先不保存当车行驶到高速路的kv imei ，因为高速路的整段路况更新方法还未确定，当由高速换到低速
        路时，高速的整段路况先不保存。在这里 先删除当前imei的路况信息，保证切换到低速路时，不产生
        路况。后面可能需要在低速模型中处理高速换到低速的情况
        init_IMEI_data(gps_info, road_info, &kv_IMEI);
        x_printf(D, "set kv_IMEI.roadID %ld\n", kv_IMEI.roadID);

        if (ERR_IMEI == set_IMEI_to_kv(&kv_IMEI)) {
                x_printf(D, "***set_IMEI_to_kv failed***\n");
                return ERR_IMEI;
        }
#endif
        return SUC_IMEI;
}
*/

int subsec_calculate( struct ev_loop * p_loop, gps_info_t *gps_info, road_info_t *road_info, struct subsec_model_t *subsec)
{      
        SECKV_ROAD      kv_road = { 0 };
        KV_IMEI         kv_IMEI = { 0 };

        int ok  = get_IMEI_from_kv(atoll( gps_info->IMEI), &kv_IMEI );
        int ret = get_roadINFO_from_kv( road_info->new_roadID, &kv_road );
#if 0
        switch( ok ) {/*在配置了single模型下，低速换高速需要产生路况，高速换低速
                        高速换高速保留和低速一致也产生路况
                        */
                case ERR_IMEI:
                        return ERR_IMEI;

                case SUC_IMEI:
                        if( kv_IMEI.roadID != road_info->rr_id ) {
                                /*if( kv_IMEI.rt == LOWWAY ) {
                                        new_road();
                                }*/
                                if( ERR_IMEI == subsec->ulike_call(p_loop, kv_IMEI, kv_roadID, gps_info, road_info, single->single_cfg) )
                                        return ERR_IMEI;

                        }
                        else {
                                if( ERR_IMEI == subsec->like_call(gps_info, road_info, kv_IMEI) )
                                        return ERR_IMEI;
                        }

                        break;
                case NIL_IMEI:
                        init_IMEI_data(gps_info, road_info, &kv_IMEI);          // new road

                        x_printf(D, "set kv_IMEI.roadID %ld\n", kv_IMEI.roadID);

                        if (ERR_IMEI == set_IMEI_to_kv(&kv_IMEI)) {
                                x_printf(D, "***set_IMEI_to_kv failed***\n");
                                return ERR_IMEI;
                        }

                        break;
                default:
                        break;
        }
#endif
        if( ERR_IMEI == ok )
                return ERR_IMEI;
        else if( SUC_IMEI == ok ) {
                if( (kv_IMEI.rt != HIGHWAY || kv_IMEI.rt != EXPRESSWAY) &&  kv_IMEI.roadID != road_info->new_roadID && (road_info->rt == HIGHWAY || road_info->rt == EXPRESSWAY) ) {
                        x_printf(D, "lowway:%ld change to EXPRESSWAY:%ld\n", kv_IMEI.roadID, road_info->new_roadID);
                        single_new_road_2(p_loop, kv_IMEI, kv_road, gps_info, road_info, subsec->subsec_cfg.road_match_limit);
                        delete_IMEI_from_kv( &kv_IMEI );
                }
        }

        switch( ret ) {
                case ERR_ROAD:
                        return ERR_ROAD;
                case SUC_ROAD:

                        //set_CURRENTROAD_data(&kv_road, gps_info, road_info, subsec->subsec_cfg.merged_speed, subsec->subsec_cfg.replace_limit);
                        subsec->update_roadsec(&kv_road, gps_info, road_info, subsec->subsec_cfg.merged_speed, subsec->subsec_cfg.replace_limit, subsec->subsec_cfg.expire_time);
                        set_secroad_to_kv(&kv_road);
                        if( kv_road.sec_num > 1 )
                                subsec->section_update(&kv_road, subsec->subsec_cfg.expire_time, p_loop);
                        break;
                case NIL_ROAD:
                        //init_SECROAD_data(gps_info, road_info, &kv_road);
                        subsec->init_roadsec(gps_info, road_info, &kv_road);
                        set_secroad_to_kv(&kv_road);
                        //subsec->section_update(&kv_road, subsec->subsec_cfg.expire_time, p_loop);
                        break;
                default:
                        break;
        }

        return SUC_ROAD;
}  
