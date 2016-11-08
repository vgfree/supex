#pragma once
#include <ctype.h>
#include "rr_def.h"
#include "rr_cfg.h"
#include "utils.h"
#include "ev.h"

/*通过gps数据解析的字段*/
typedef struct gps_info_t {
        unsigned short max_speed;       /*gps包中最大速度*/
        unsigned short min_speed;        /*gps包中最小速度*/
        unsigned short avg_speed;        /*gps包中平均速度*/
        long start_time;                        /*gps包开始时间*/
        long end_time;                         /*gps包结束时间*/
        double longitude;                     /*gps最新点经度*/
        double latitude;                        /*gps最新点纬度*/
        short direction;                         /*gps最新点方向角*/
        short altitude;
        char IMEI[IMEI_LEN + 1];           /*当前设备IMEI*/
        char tokenCode[IMEI_LEN+1];        /*当前tokenCode*/
} gps_info_t;

/*
描述：解析gps数据，过滤gps数据，若是测试帐号测转发。
参数 loop（in）：ev_loop指针
       p_data（in）：gps json数据包
       p_gps（out）：存放gps解析后数据
返回值：
       0：解析成功
       -1：解析失败
       -2：测试帐号数据已转发
       -3:  gps已经被过滤
*/
int gps_decode(struct ev_loop *loop, const char *p_data, gps_info_t *p_gps, rr_link forward_link);
