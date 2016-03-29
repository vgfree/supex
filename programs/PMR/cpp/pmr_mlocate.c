#include "pmr_mlocate.h"
#include "map_pmr.h"
#include "entry.h"
#include "pmr_utils.h"
#include "map_utils.h"
#include "map_seg.h"
#include <string.h>
#include "user_info.h"
#include "map_grid.h"
#include "match_lines.h"
#include "user_info.h"
#include "libmini.h"
#include "map_utils.h"


static int check_parameter_mlocate(struct data_node *p_node, gps_point_t *p_point, char *p_imei, int size)
{
        if (!p_node || !p_node->recv.buf_addr) {
                return -1;
        }

        struct net_cache        *p_cache = &p_node->send;
        struct redis_status     *p_rst = &p_node->redis_info.rs;
        char                    *p_buf = p_node->recv.buf_addr;

        if (p_rst->keys != 7 || !p_point) {
                send_error(p_cache, "paramters error");
                return -1;
        }

         strncpy(p_imei, p_buf + p_rst->key_offset[0], p_rst->klen_array[0]);

         double lon = strtod(p_buf + p_rst->key_offset[1], NULL);
         if ((lon > 180.0) || (lon < -180.0)) {
                 send_error(p_cache, "longitude error");
                 return -1;
         }
         p_point->lon = lon;

         double lat = strtod(p_buf + p_rst->key_offset[2], NULL);
         if ((lat > 90.0) || (lat < -90.0)) {
                 send_error(p_cache, "latitude error");
                 return -1;
         }
         p_point->lat = lat;

         long dir = strtol(p_buf + p_rst->key_offset[3], NULL, 0);
         if ((dir > 360.0) || (dir < -1.0)) {
                 send_error(p_cache, "direction error");
                 return -1;
         }
         p_point->dir = (short)dir;

         long alt = strtol(p_buf + p_rst->key_offset[4], NULL, 0);
         if ((alt >8848) || (alt < -200)) {
                 send_error(p_cache, "altitude error");
                 return -1;
         }
         p_point->alt = (short)alt;

         long speed = strtol(p_buf + p_rst->key_offset[5], NULL, 0);
         if ((speed >200) || (speed < 0)) {
                 send_error(p_cache, "speed error");
                 return -1;
         }
        p_point->speed = (short)speed;

        long long time = strtoll(p_buf + p_rst->key_offset[6], NULL, 0);
        if (time <= 0) {
                send_error(p_cache, "time error");
                return -1;
        }
       p_point->time = time;

        return 0;
}

/*将BL扩展，用于BL筛选*/
static void extent_coor(double coor1, double coor2, double *p_min_coor, double *p_max_coor)
{
        double  min_coor = (coor1 > coor2) ? coor2 : coor1;
        double  max_coor = (coor1 > coor2) ? coor1 : coor2;

        *p_min_coor = min_coor - GRID_EXTEND_LIMIT;
        *p_max_coor = max_coor + GRID_EXTEND_LIMIT;
}

static int check_sg_dist(match_road_t *p_match, map_seg_info *p_sg ,short dist)
{
    int i;
    for(i = 0; i < p_match->match_size; i++) {
        if(p_sg == p_match->roads[i].p_sg) {
            if(dist < p_match->roads[i].dist)
                p_match->roads[i].dist = dist;

            return 0;
        }
    }

    return -1;
}

/*点在路上定位回调函数*/
static int pmr_mlocate_cb(void *arg, map_line_info *p_line_info)
{
        if (!arg || !p_line_info) {
                return -1;
        }

        short sub_dir = 0;
        short dist = 0;
        match_road_t *p_match = (match_road_t *)arg;

        /*方向角筛选*/
        if (p_match->cur_pt.dir >= 0)  {
                sub_dir = direction_sub(p_match->cur_pt.dir, p_line_info->dir) ;
                if( sub_dir > PMR_MAX_SUB_DIR)
                    return -1;
        }

        /*BL筛选*/
        double  min_lon = 0;
        double  max_lon = 0;
        double  min_lat = 0;
        double  max_lat = 0;

        extent_coor(p_line_info->start_lon, p_line_info->end_lon, &min_lon, &max_lon);
        extent_coor(p_line_info->start_lat, p_line_info->end_lat, &min_lat, &max_lat);

        if ((p_match->cur_pt.lon < min_lon) || (p_match->cur_pt.lon > max_lon)
                || (p_match->cur_pt.lat < min_lat) || (p_match->cur_pt.lat > max_lat)) {
                return -1;
        }

        /*距离筛选*/
        dist = dist_p2l(p_match->cur_pt.lon, p_match->cur_pt.lat, p_line_info->start_lon, p_line_info->start_lat,
                        p_line_info->end_lon, p_line_info->end_lat);

        if (dist > PMR_MAX_DIST) {
                return -1;
        }

        if(0 < is_in_line_range(p_match->cur_pt.lon, p_match->cur_pt.lat,
                            p_line_info->start_lon, p_line_info->start_lat, p_line_info->end_lon, p_line_info->end_lat)) {
        	x_printf(I, "line:%d%03d, point not in line range");
                return -1;
	}

        if ((p_match->match_size + 1) >= MAX_MATCH_ROAD_SIZE) {
                x_printf(E, "match to many lines!");
                return 0;
        }

        x_printf(I, "line:%d%03d, dist:%d, p_lon:%f,p_lat:%f,sl:%f,sb:%f, el:%f, eb:%f\n", p_line_info->rr_id, p_line_info->sgid, dist, p_match->cur_pt.lon, p_match->cur_pt.lat,
                p_line_info->start_lon, p_line_info->start_lat,p_line_info->end_lon, p_line_info->end_lat);

        back_seg ret_seg;
        if(0 != map_seg_query(p_line_info->rr_id, p_line_info->sgid,&ret_seg)) {
                return -1;
        }

        //检查该sgid是否已经匹配过了
        if(0 != check_sg_dist(p_match, ret_seg.ptr_seg, dist)) {
            p_match->roads[p_match->match_size].p_sg = ret_seg.ptr_seg;
            p_match->roads[p_match->match_size].p_sg_name = ret_seg.ptr_name;
            p_match->roads[p_match->match_size].dist = dist;
            p_match->roads[p_match->match_size].sub_dir = sub_dir;
            p_match->match_size++;
        }

        return 0;
}

int entry_cmd_mlocate(struct data_node *p_node)
{
        if (!p_node) {
                return 0;
        }

        struct net_cache        *p_cache = &p_node->send;
        char imei[IMEI_LEN + 1] = {0};
        gps_point_t gps_pt;

        if (0 != check_parameter_mlocate(p_node, &gps_pt, imei, IMEI_LEN)) {
                return 0;
        }

        x_printf(I, "time:%lld,lon:%f, lat:%f, dir:%d, alt:%d,spd:%d\n",gps_pt.time,
                 gps_pt.lon, gps_pt.lat, gps_pt.dir, gps_pt.alt, gps_pt.speed );

        match_road_t match_road;
        memset(&match_road, 0, sizeof(match_road_t));
        match_road.cur_pt = gps_pt;

        int ret = map_grid_query(gps_pt.lon, gps_pt.lat, pmr_mlocate_cb, (void *)&match_road);
        if(ret <= 0) {
                cache_add(p_cache, OPT_MULTI_BULK_NULL, strlen(OPT_MULTI_BULK_NULL));
                return 0;
        }

        int idx = match_roads(&match_road, imei);
        if(idx < 0  ) {
                cache_add(p_cache, OPT_MULTI_BULK_NULL, strlen(OPT_MULTI_BULK_NULL));
                return 0;
        }

        road_info_t *p_road = &(match_road.roads[idx]);

        // *2\r\n$6\r\nlineID\r\n$5\r\n12345\r\n
        cache_add(p_cache, "*", 1);
        put_number_out(p_cache, 11);

        cache_add(p_cache, "$", 1);
        put_number_out(p_cache, get_number_len(p_road->p_sg->rrid));
        put_number_out(p_cache, p_road->p_sg->rrid);

        cache_add(p_cache, "$", 1);
        put_number_out(p_cache, get_number_len(p_road->p_sg->sgid));
        put_number_out(p_cache, p_road->p_sg->sgid);

        // TFID

        cache_add(p_cache, "$", 1);
        put_number_out(p_cache, get_number_len(p_road->p_sg->sgid));
        put_number_out(p_cache, p_road->p_sg->sgid);

        // countyCode  TODO  need init the data file
        cache_add(p_cache, "$", 1);
        put_number_out(p_cache, get_number_len(p_road->p_sg->countyCode));
        put_number_out(p_cache, p_road->p_sg->countyCode);

        // RT
        cache_add(p_cache, "$", 1);
        put_number_out(p_cache, get_number_len(p_road->p_sg->sgid_rt));
        put_number_out(p_cache, p_road->p_sg->sgid_rt);

        // roadName
        cache_add(p_cache, "$", 1);
        if(!p_road->p_sg_name || strlen(p_road->p_sg_name) == 0) {
                put_number_out(p_cache, 0);
                put_string_out(p_cache, "");
        } else {
                put_number_out(p_cache, strlen(p_road->p_sg_name));
                put_string_out(p_cache, p_road->p_sg_name);
        }

        //起始经纬度
        put_double_out(p_cache, p_road->p_sg->start_lon);
        put_double_out(p_cache, p_road->p_sg->start_lat);
        put_double_out(p_cache, p_road->p_sg->end_lon);
        put_double_out(p_cache, p_road->p_sg->end_lat);

        cache_add(p_cache, "$", 1);
        put_number_out(p_cache, get_number_len(p_road->p_sg->length));
        put_number_out(p_cache, p_road->p_sg->length);

        return 0;
}
