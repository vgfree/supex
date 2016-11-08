/*
 * *@author: shumenghui@mirrtalk.com
 * *@date: 2016_0400
 * *@description: 分段路况
 */

#include "subsection_model_v2.h"
#include "async_tasks/async_api.h"
#include "rr_def.h"
#include "rr_cfg.h"
#include "gps_info.h"
#include "road_info.h"
#include "single_model.h"
#include "utils.h"

#include <math.h>
#include <time.h> 
#include <string.h>

#define NEAR_TIME 180
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

static void set_end_roadsection( SITE *sec, unsigned short avg, long endtime, unsigned short max )
{
        sec->endtime = endtime;
        sec->avg_speed = avg;
        sec->max_speed = max;
}

static void set_roadsection(SITE *sec, unsigned short begin, unsigned short end, unsigned short avg, unsigned short max, long endtime, double lon, double lat, char *imei)
{
        sec->begin = begin;
        strncpy(sec->imei, imei, IMEI_LEN);
        if( end )
                sec->end   = end;
        sec->avg_speed = avg;
        sec->max_speed = max;
        if( endtime )
                sec->endtime = endtime;
        if( lon )
                sec->longitude = lon;
        if( lat )
                sec->latitude = lat;
}

static void set_used_time( SITE *sec, int flag, unsigned short used )
{
        if( flag )
                sec->used_time = used;
        else {
                sec->used_time += used;
        }
}
/*
 * 比较前一段路况更新时间与GPS时间
 * @param  front：前段路况更新时间 back：当前GPS时间 fac：阈值
 * @return 2：front比back新 1：在阈值内 0：路况过期
 * */
static int comparing_front_and_back_t(long front, long back, int fac)
{
        long dis = back - front;
        if(dis < fac) {
                if(dis < 0)
                        return 2;
                else return 1;
        }
        else return 0;
}

/*
 * 删除超过过期时间的路况 以下注释n=new o=old N=now b=back
 * */
static int delete_roadsection(SECKV_ROAD *kv_road, int B, int E, long now_time, int expire_time)
{
        int num = 0;
        SITE *sec = kv_road->road_sec;
        if( B > E || E > kv_road->sec_num) {
                return -1;
        }
        int i, j;
        for( i = B; i < E + 1; i++ ) {
                int ret = comparing_front_and_back_t(sec[B].endtime, now_time, expire_time);
                /*
                 * 发现过期路况 删除 过期点到E之间的路况
                 * */
                if(0 == ret){
                        for(j=i; j<kv_road->sec_num; j++) {
                                set_roadsection( &sec[j], sec[j+1].begin, sec[j+1].end, sec[j+1].avg_speed, sec[j+1].max_speed, sec[j+1].endtime, sec[j+1].longitude, sec[j+1].latitude, sec[j+1].imei );
                                set_used_time(&sec[j], 1, sec[j+1].used_time);
                        }
                        num++;
                        kv_road->sec_num -= 1;
                }
        }
        return num;
}

static void expend_roadsection(SITE *sec, int B, int num)
{
        if( B > num ) {
                x_printf(W, "expend sec error B >= num\n");
                return;
        }
        int i;
        for( i = num; i >= B; i-- ) {
                set_roadsection( &sec[i], sec[i-1].begin, sec[i-1].end, sec[i-1].avg_speed, sec[i-1].max_speed, sec[i-1].endtime, sec[i-1].longitude, sec[i-1].latitude, sec[i-1].imei );
        }
}

void init_SECROAD_data(gps_info_t *gps_info, road_info_t *road_info, SECKV_ROAD *kv_road)
{
        strncpy(kv_road->IMEI, gps_info->IMEI, IMEI_LEN);
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

//分段道路初始化
void init_SECROAD_data2(gps_info_t *gps_info, road_info_t *road_info, SECKV_ROAD *kv_road, int max_limit)
{
        strncpy(kv_road->IMEI, gps_info->IMEI, IMEI_LEN);
        kv_road->old_roadID = road_info->new_roadID;
        kv_road->end_time = gps_info->end_time;
        kv_road->avg_speed = gps_info->avg_speed;
        kv_road->max_speed = gps_info->max_speed;
        kv_road->used_time = gps_info->end_time - gps_info->start_time;
        kv_road->citycode = road_info->city_code;
        kv_road->countycode = road_info->county_code; 
        
        //设置起点
        kv_road->road_sec[98].end = 0;
        kv_road->road_sec[98].begin = 0;
        strncpy(kv_road->road_sec[98].imei, kv_road->IMEI, IMEI_LEN);
        //kv_road->road_sec[98].avg_speed = gps_info->avg_speed;
        //kv_road->road_sec[98].endtime = gps_info->end_time;
        kv_road->road_sec[98].longitude = road_info->start_lon;
        kv_road->road_sec[98].latitude =  road_info->start_lat;
        //设置终点
        kv_road->road_sec[99].end = road_info->len;
        kv_road->road_sec[99].begin = 0;
        strncpy(kv_road->road_sec[99].imei, kv_road->IMEI, IMEI_LEN);
        kv_road->road_sec[99].avg_speed = gps_info->avg_speed;
        kv_road->road_sec[99].max_speed = gps_info->max_speed;
        kv_road->road_sec[99].endtime = gps_info->end_time;
        kv_road->road_sec[99].longitude = road_info->end_lon;
        kv_road->road_sec[99].latitude  = road_info->end_lat;

//出现在道路的初始距离 超过最大值 不予分段
        unsigned short end = get_begin_lenth(gps_info->longitude, gps_info->latitude, road_info->start_lon, road_info->start_lat, road_info->end_lon, road_info->end_lat, road_info->len);
        if( end > max_limit) {
                x_printf(W, "drift point lon:%f lat:%f dis:%d\n", gps_info->longitude, gps_info->latitude, end);
                return;
        }

        kv_road->sec_num = 1;
        kv_road->road_sec[0].end = end;
        kv_road->road_sec[0].begin = 0;
        strncpy(kv_road->road_sec[0].imei, kv_road->IMEI, IMEI_LEN);
        kv_road->road_sec[0].avg_speed = gps_info->avg_speed;
        kv_road->road_sec[0].max_speed = gps_info->max_speed;
        kv_road->road_sec[0].endtime = gps_info->end_time;
        kv_road->road_sec[0].used_time = gps_info->end_time - gps_info->start_time;
        kv_road->road_sec[0].longitude = gps_info->longitude;
        kv_road->road_sec[0].latitude = gps_info->latitude;

        x_printf(D, "init highway road sec B:%d E:%d A:%d\n", kv_road->road_sec[0].begin, kv_road->road_sec[0].end, kv_road->road_sec[0].avg_speed);
}

static void set_road_data(gps_info_t *gps_info, road_info_t *road_info, SECKV_ROAD *kv_road )
{
        strncpy(kv_road->IMEI, gps_info->IMEI, IMEI_LEN);
        kv_road->end_time = gps_info->end_time;
        kv_road->avg_speed = (gps_info->avg_speed + kv_road->avg_speed) / 2;
        kv_road->max_speed = MAX( gps_info->max_speed, kv_road->max_speed);
        kv_road->used_time = gps_info->end_time - gps_info->start_time + kv_road->used_time;
        kv_road->old_roadID = road_info->new_roadID;
}

static int get_interval( SECKV_ROAD *kv_road, int B, int x )
{
        int i = 0;
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
/*
 * @param
 * @return 
 * */
typedef long long ll;
inline int is_same_imei(ll r_imei, ll n_imei)
{
        return (r_imei == n_imei) ? 1 : 0;
}

//更新道路分段信息
void update_roadsection( SECKV_ROAD *kv_road, gps_info_t *gps_info, road_info_t *road_info, int merged_speed_limit_l, int merged_speed_limit_h, int replace_limit, int expire_time )
{
        int B = get_begin_lenth(gps_info->longitude, gps_info->latitude, road_info->start_lon, road_info->start_lat, road_info->end_lon, road_info->end_lat, road_info->len);
        unsigned short avg_speed = gps_info->avg_speed;
        unsigned short max_speed = gps_info->max_speed;
        unsigned short used_time = gps_info->end_time - gps_info->start_time;
        SITE *road_sec = kv_road->road_sec;
        long et = gps_info->end_time;
        char imei[IMEI_LEN] = "";
        strncpy(imei,gps_info->IMEI,IMEI_LEN);
        int num     = kv_road->sec_num;//段数
        int retnumB = get_interval(kv_road, 0, B);//所在段
        int merged_speed = 0;
        if( road_sec[retnumB - 1].avg_speed < 15 )
                merged_speed = merged_speed_limit_l;
        else
                merged_speed = merged_speed_limit_h;

        if(num == 0 && B < 400) {
                kv_road->sec_num = 1;
                set_roadsection(&kv_road->road_sec[0], 0, B, avg_speed, max_speed, et, gps_info->longitude, gps_info->latitude, imei);
                set_end_roadsection(&kv_road->road_sec[99], avg_speed, et, max_speed);
                set_used_time( &kv_road->road_sec[0], 1, used_time );
                set_road_data(gps_info, road_info, kv_road);
                x_printf(W, "section num = 0\n");
                return;
        }

        if( num >= ROADSECNUM) {  
                x_printf(E, "road sec num is over ! num:%d rr_id:%d sg_id:%d", num, road_info->rr_id, road_info->sg_id);
                return;       
        } 
        x_printf(D, "sec num: %d\n", num);
        
        //车行驶到所有段的前方
        if( retnumB == num ) {
                // 道路起始只有两段短距离的路段 车直接越过两段，这时需删除过期路段，由于比较的是kv_road->end_time道路时间，当一辆车在路上一直堵着，
                // kv_road->end_time会不断更新，则不会进入此条件内
                if(!comparing_front_and_back_t(kv_road->end_time, et, expire_time)) {
                        int ret_sec_num = delete_roadsection(kv_road, 0, num-1, et, expire_time);
                        if(ret_sec_num > 0) {
                                int secnum = num - ret_sec_num;
                                set_roadsection(&kv_road->road_sec[secnum], 0, B, avg_speed, max_speed, et, gps_info->longitude, gps_info->latitude, imei);
                                set_used_time( &kv_road->road_sec[secnum], 1, used_time );
                                set_end_roadsection(&kv_road->road_sec[99], avg_speed, et, max_speed);
                                x_printf(D, "sec num: %d del and add road sec B:%d A:%d\n", num, B, avg_speed);
                                set_road_data(gps_info, road_info, kv_road);
                                return;
                        }
                }

                // 获取当前点和上点的距离
                int Distance = dist_p2p(road_sec[retnumB-1].longitude, road_sec[retnumB-1].latitude, gps_info->longitude, gps_info->latitude);
                // 距离前段时间 平均速度 距离 满足要求 --> 合并
                if( comparing_d(abs(road_sec[retnumB - 1].avg_speed - avg_speed), merged_speed) && comparing_d( Distance, replace_limit ) ) {
                        if( num > 1 ) {
                                avg_speed = (kv_road->road_sec[retnumB - 1].avg_speed + avg_speed) / 2;
                                set_roadsection(&kv_road->road_sec[retnumB-1], 0, B, avg_speed, max_speed, et, gps_info->longitude, gps_info->latitude, imei);
                                set_used_time( &kv_road->road_sec[retnumB-1], 0, used_time );
                                set_end_roadsection(&kv_road->road_sec[99], avg_speed, et, max_speed);
                                x_printf(D, "sec num: %d merged road sec B:%d A:%d\n", num, B, avg_speed);
                        }
                        else {
                                kv_road->sec_num += 1;
                                avg_speed = (kv_road->road_sec[retnumB - 1].avg_speed + avg_speed) / 2;
                                set_roadsection(&kv_road->road_sec[retnumB], 0, B, avg_speed, max_speed, et, gps_info->longitude, gps_info->latitude, imei);
                                set_used_time( &kv_road->road_sec[retnumB], 1, used_time );
                                set_end_roadsection(&kv_road->road_sec[99], avg_speed, et, max_speed);
                                x_printf(D, "sec num: %d merged add road sec B:%d A:%d\n", num, B, avg_speed);
                        
                        }
                }
                else { //分段
                        kv_road->sec_num += 1;
                        set_roadsection(&kv_road->road_sec[retnumB], 0, B, avg_speed, max_speed, et, gps_info->longitude, gps_info->latitude, imei);
                        set_used_time( &kv_road->road_sec[retnumB], 1, used_time );
                        set_end_roadsection(&kv_road->road_sec[99], avg_speed, et, max_speed);
                        x_printf(D, "sec num: %d add road sec B:%d A:%d\n", num, B, avg_speed);
                }
                set_road_data(gps_info, road_info, kv_road);
        }
        // 同时有其他车进入分段区间内 或 下一阶段有车更新路况
        else {
                x_printf(D, "replace retnumB : %d\n", retnumB);
                if( retnumB > 0 )
                        retnumB -= 1;

                // 当处于最后一段 更新最后一段的路况
                if( retnumB == num - 2 || num == 1)
                        set_end_roadsection(&kv_road->road_sec[99], avg_speed, et, max_speed);

                if( 0 == comparing_front_and_back_t(road_sec[retnumB].endtime, et, expire_time) ) {
                        int ret_sec_num = delete_roadsection(kv_road, 0, retnumB, et, expire_time);
                        if(ret_sec_num > 0) {
                                int secnum = retnumB - ret_sec_num + 1;
                                set_roadsection(&kv_road->road_sec[secnum], 0, B, gps_info->avg_speed, max_speed, et, gps_info->longitude, gps_info->latitude, imei);
                                set_used_time( &kv_road->road_sec[secnum], 1, used_time );
                                x_printf(D, "sec num: %d expire replace road sec B:%d A:%d\n", num, B, avg_speed);
                                set_road_data(gps_info, road_info, kv_road);
                                return;
                        }
                }
                
                int Distance = dist_p2p(road_sec[retnumB].longitude, road_sec[retnumB].latitude, gps_info->longitude, gps_info->latitude);
                if( comparing_d( Distance, replace_limit ) && comparing_d(abs(road_sec[retnumB].avg_speed - avg_speed), merged_speed) ) {
                        avg_speed = (road_sec[retnumB].avg_speed + avg_speed) / 2;
                        set_roadsection(&kv_road->road_sec[retnumB], 0, B, avg_speed, max_speed, et, gps_info->longitude, gps_info->latitude, imei);
                        set_used_time( &kv_road->road_sec[retnumB], 0, used_time );
                        x_printf(D, "sec num: %d overlay road sec B:%d A:%d distance:%d\n", num, B, avg_speed, Distance);
                        
                }
                else {
                        expend_roadsection(kv_road->road_sec, retnumB + 1, num);
                        kv_road->sec_num += 1;
                        set_roadsection(&kv_road->road_sec[retnumB+1], 0, B, avg_speed, max_speed, et, gps_info->longitude, gps_info->latitude, imei);
                        set_used_time( &kv_road->road_sec[retnumB+1], 1, used_time );
                        x_printf(D, "sec num: %d split road sec B:%d A:%d distabce:%d replace_limit%d\n",num, B, avg_speed, Distance, replace_limit);
                }
        }
        set_road_data(gps_info, road_info, kv_road);
}

int subsec_calculate( struct ev_loop * p_loop, gps_info_t *gps_info, road_info_t *road_info, struct subsec_model_t *subsec)
{      
        SECKV_ROAD      *kv_road = NULL;
        KV_IMEI         kv_IMEI = { 0 };

        //高速 和 低速 对应不同的replace_limit（超过replace_limit需再增加一段）
        unsigned short replace_limit = subsec->subsec_cfg.replace_limit_l;
        if( road_info->rt == HIGHWAY || road_info->rt == EXPRESSWAY )
                replace_limit = subsec->subsec_cfg.replace_limit_h;

        int ok  = get_IMEI_from_kv( gps_info->IMEI, &kv_IMEI );
        int ret = roadsec_info_get( road_info->new_roadID, &kv_road );
        if( ERR_IMEI == ok )
                return ERR_IMEI;
        else if( SUC_IMEI == ok ) {
                //判断是否由低速路转入高速分段模型 
                if( (kv_IMEI.rt != HIGHWAY || kv_IMEI.rt != EXPRESSWAY) &&  kv_IMEI.roadID != road_info->new_roadID && (road_info->rt == HIGHWAY || road_info->rt == EXPRESSWAY) ) {
                        x_printf(D, "lowway:%ld change to EXPRESSWAY:%ld\n", kv_IMEI.roadID, road_info->new_roadID);
                        single_new_road_2(p_loop, kv_IMEI, *kv_road, gps_info, road_info, subsec->subsec_cfg.road_match_limit);
                        //清除imei
                        delete_IMEI_from_kv( &kv_IMEI );
                }
        }

        switch( ret ) {
                case ERR_ROAD:
                        return ERR_ROAD;
                case SUC_ROAD:
                        AO_Lock(&kv_road->locker);
                        subsec->update_roadsec(kv_road, gps_info, road_info, subsec->subsec_cfg.merged_speed_l, subsec->subsec_cfg.merged_speed_h, replace_limit, subsec->subsec_cfg.expire_time);
                        if( kv_road->sec_num > 1 )
                                subsec->section_update(kv_road, subsec->subsec_cfg.expire_time, p_loop);
                        AO_Unlock(&kv_road->locker);
                        break;
                case NIL_ROAD:
                        AO_Lock(&kv_road->locker);
                        subsec->init_roadsec(gps_info, road_info, kv_road, subsec->subsec_cfg.init_max);
                        AO_Unlock(&kv_road->locker);
                        break;
                default:
                        break;
        }

        return SUC_ROAD;
}  
