#include "calculate.h"
#include "rr_cfg.h"
#include "utils.h"
#include "kv_road.h"
#include "kv_imei.h"
#include "match_road.h"
#include "async_tasks/async_api.h"
#include "pools/xpool.h"
#include "pool_api/conn_xpool_api.h"
#include "redis_api/redis_status.h"
#include "redis_parse.h"

#include <time.h>

#define ASYNC_LIBEV_THRESHOLD   6
#define ASYNC_LUAKV_THRESHOLD   1
#define BUFF_USE_LEN            10240
#define REDIS_ERR       -1
#define REDIS_OK        0

extern struct rr_cfg_file g_rr_cfg_file;

#if 0
static void http_str(char *http_data, ROAD_INFO *road_info, char *host, short port, KV_ROADID *kv_roadID)
{
	char buff[1024] = { 0 };

	snprintf(buff, 1023,
		"{\"RRID\":%ld, \"SGID\":%d,\"maxSpeed\":%d, \"avgSpeed\":%d, \"collectTime\":%ld,\"passTime\":%ld}",
		road_info->road_rootID, road_info->segmentID, kv_roadID->max_speed,
		kv_roadID->avg_speed, kv_roadID->end_time, kv_roadID->used_time);
	char HTTP_FORMAT[] = "POST /%s HTTP/1.1\r\n"
		"User-Agent: curl/7.33.0\r\n"
		"Host: %s:%d\r\n"
		"Content-Type: application/json; charset=utf-8\r\n"
		"Connection:%s\r\n"
		"Content-Length:%d\r\n"
		"Accept: */*\r\n\r\n%s";
	snprintf(http_data, 2047, HTTP_FORMAT, "luakv_roadRank", host, port, "Keep-Alive", strlen(buff), buff);
}

static int snd_data_to_luakv(char *http_data, struct ev_loop *loop, char *host, int port)
{
	struct async_ctx        *ac = NULL;
	struct cnt_pool         *cpool = NULL;

	ac = async_initial(loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, 1);

	if (ac) {
		void    *sfd = (void *)(intptr_t)-1;
		int     rc = conn_xpool_gain(&cpool, host, port, &sfd);

		if (rc) {
			async_distory(ac);
			return -1;
		}

		async_command(ac, PROTO_TYPE_HTTP, (intptr_t)(void *)sfd, cpool, NULL, NULL, http_data, strlen(http_data));
		async_startup(ac);
	}

	return 0;
}
#endif	/* if 0 */
/*
static int add_redis_task(char *redis_buff, struct rr_link *link, struct async_ctx *ac)
{
	char *proto = NULL;

	cmd_to_proto(&proto, redis_buff);

	if (!proto) {
		x_printf(E, "add_redis_task, cmd_to_proto error");
		return -1;
	}

	struct cnt_pool *cpool = NULL;

	void    *sfd = (void *)(intptr_t)-1;
	int     rc = conn_xpool_gain(&cpool, link->host, link->port, &sfd);

	if (rc) {
		async_distory(ac);
		return -1;
	}

	if (proto) {
		async_command(ac, PROTO_TYPE_REDIS, (void *)(intptr_t)sfd, cpool, NULL, NULL,
			proto, strlen(proto));
		free(proto);
	}

	return 0;
}
*/
static int add_redis_task(char command[], struct rr_link *link, struct ev_loop *loop)
{       
        struct xpool            *cpool = conn_xpool_find(link->host, link->port);
        struct async_api        *api = async_api_initial(loop, 1, true, QUEUE_TYPE_FIFO, NEXUS_TYPE_TEAM, NULL, NULL, NULL);
        
        // x_printf(D,"import_to_redis: %s\n",command);
        
        if (api && cpool) {
                /*data*/
                char    *proto;
                int     ok = cmd_to_proto(&proto, command);
        
                if (ok == REDIS_ERR) {
                        async_api_distory(api);
                        return -1;
                }
        
                /*send*/
                struct command_node *cmd = async_api_command(api, PROTO_TYPE_REDIS, cpool, proto, strlen(proto), NULL, NULL);
                free(proto);
        
                if (cmd == NULL) {
                        async_api_distory(api);
                        return -1;
                }
        
                async_api_startup(api);
                return 0;
        }
        
        return -1;
}

static int update_redis(KV_ROADID *kv_roadID, ROAD_INFO *road_info, struct ev_loop *loop)
{
	int     old_rr_id = kv_roadID->old_roadID / 1000;
	int     old_sg_id = kv_roadID->old_roadID % 1000;
	int     save_time = g_rr_cfg_file.save_time;

	/*添加单条道路路况*/
	char redis_buff[BUFF_USE_LEN] = { 0 };
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"SET %ld:roadSpeedInfo %d@%d@%ld@%ld@%s",
		kv_roadID->old_roadID, kv_roadID->max_speed, kv_roadID->avg_speed,
		kv_roadID->end_time, kv_roadID->used_time, kv_roadID->IMEI);
	x_printf(D, "redis command: %s", redis_buff);

	add_redis_task(redis_buff, &(g_rr_cfg_file.road_traffic_server), loop);

	/*删除过期城市道路路况*/
	time_t now_time;
	time(&now_time);
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"ZREMRANGEBYSCORE %d:cityinfo -inf %ld",
		kv_roadID->citycode, now_time - save_time);
	add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), loop);

	/*添加城市道路路况*/
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"ZADD %d:cityinfo %ld %d:%d:%d:%d:%ld:%ld",
		kv_roadID->citycode, kv_roadID->end_time, old_rr_id, old_sg_id, kv_roadID->max_speed,
		kv_roadID->avg_speed, kv_roadID->used_time, kv_roadID->end_time);
	add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), loop);

	/*删除过期区县道路路况*/
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"ZREMRANGEBYSCORE %d:countyinfo -inf %ld",
		kv_roadID->countycode, now_time - save_time);
	add_redis_task(redis_buff, &(g_rr_cfg_file.county_traffic_server), loop);

	/*添加城市道路路况*/
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"ZADD %d:countyinfo %ld %d:%d:%d:%d:%ld:%ld",
		kv_roadID->countycode, kv_roadID->end_time, old_rr_id, old_sg_id, kv_roadID->max_speed,
		kv_roadID->avg_speed, kv_roadID->used_time, kv_roadID->end_time);
	add_redis_task(redis_buff, &(g_rr_cfg_file.county_traffic_server), loop);

	return 0;
}

static int set_IMEI_data(GPS_INFO *gps_info, ROAD_INFO *road_info, KV_IMEI *kv_IMEI)
{
	kv_IMEI->count += 1;
	kv_IMEI->max_speed_num += gps_info->max_speed;
	kv_IMEI->max_speed = MAX(kv_IMEI->max_speed, gps_info->max_speed);

	if (kv_IMEI->end_time < gps_info->end_time) {
		kv_IMEI->end_time = gps_info->end_time;
	} else {
		return ERR_IMEI;
	}

	kv_IMEI->direction = gps_info->direction;
	return SUC_IMEI;
}

static void set_roadID_data(KV_IMEI *kv_IMEI, KV_ROADID *kv_roadID)
{
	strcpy(kv_roadID->IMEI, kv_IMEI->IMEI);
	kv_roadID->old_roadID = kv_IMEI->roadID;
	kv_roadID->avg_speed = kv_IMEI->max_speed_num / kv_IMEI->count;

	if (kv_roadID->avg_speed == 0) {
		kv_roadID->avg_speed = 1;
	}

	kv_roadID->max_speed = kv_IMEI->max_speed;
	kv_roadID->end_time = kv_IMEI->end_time;
	kv_roadID->used_time = kv_IMEI->end_time - kv_IMEI->str_time;
	kv_roadID->citycode = kv_IMEI->citycode;
	kv_roadID->countycode = kv_IMEI->countycode;
}

static void init_IMEI_data(GPS_INFO *gps_info, ROAD_INFO *road_info, KV_IMEI *kv_IMEI)
{
	kv_IMEI->count = 1;
	kv_IMEI->max_speed_num = gps_info->max_speed;
	kv_IMEI->max_speed = gps_info->max_speed;
	kv_IMEI->str_time = gps_info->start_time;
	kv_IMEI->end_time = gps_info->end_time;
	kv_IMEI->roadID = road_info->new_roadID;
	strcpy(kv_IMEI->IMEI, gps_info->IMEI);
	kv_IMEI->citycode = road_info->citycode;
	kv_IMEI->countycode = road_info->countycode;
	kv_IMEI->direction = gps_info->direction;
}

int calculate(GPS_INFO *gps_info, void *data, struct ev_loop *loop)
{
	ROAD_INFO       *road_info = (ROAD_INFO *)data;
	KV_IMEI         kv_IMEI = { 0 };
	KV_ROADID       kv_roadID = { 0 };
	int             ok = get_IMEI_from_kv(gps_info->IMEI, &kv_IMEI);// get redis_IMEI data based on IMEI

	x_printf(D, "get kv_IMEI.roadID %ld\n", kv_IMEI.roadID);
	x_printf(D, "calculate gps endtime:%ld\n", gps_info->end_time);

	switch (ok)
	{
		case ERR_IMEI:
			return ERR_IMEI;

		case SUC_IMEI:

			if (kv_IMEI.roadID != road_info->new_roadID) {	// change road
				if (kv_IMEI.count >= g_rr_cfg_file.road_match_limit) {
					set_roadID_data(&kv_IMEI, &kv_roadID);

					if (ERR_IMEI == set_roadID_to_kv(&kv_roadID)) {
						x_printf(D, "***set_roadID_to_kv failed***\n");
						return ERR_IMEI;
					}

					x_printf(D,
						"kv_roadID IMEI %s shift %c max_speed %d avg_speed %d "
						"end_time %ld used_time %ld old_roadID %ld citycode %d "
						"countycode %d",
						kv_roadID.IMEI, kv_roadID.shift, kv_roadID.max_speed,
						kv_roadID.avg_speed, kv_roadID.end_time,
						kv_roadID.used_time, kv_roadID.old_roadID,
						kv_roadID.citycode, kv_roadID.countycode);

					update_redis(&kv_roadID, road_info, loop);
				}

				init_IMEI_data(gps_info, road_info, &kv_IMEI);
			} else {
				if (ERR_IMEI == set_IMEI_data(gps_info, road_info, &kv_IMEI)) {	// same road
					x_printf(E, "***gps_info package sequence error***\n");
					return ERR_IMEI;
				}
			}

			x_printf(D, "set kv_IMEI.roadID %ld\n", kv_IMEI.roadID);

			if (ERR_IMEI == set_IMEI_to_kv(&kv_IMEI)) {
				x_printf(D, "***set_IMEI_to_kv failed***\n");
				return ERR_IMEI;
			}

			break;

		case NIL_IMEI:
			init_IMEI_data(gps_info, road_info, &kv_IMEI);		// new road

			x_printf(D, "set kv_IMEI.roadID %ld\n", kv_IMEI.roadID);

			if (ERR_IMEI == set_IMEI_to_kv(&kv_IMEI)) {
				x_printf(D, "***set_IMEI_to_kv failed***\n");
				return ERR_IMEI;
			}

			break;

		default:
			break;
	}
	return SUC_IMEI;
}

