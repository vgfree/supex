#include "single_model.h"
#include "rr_def.h"
#include "rr_cfg.h"
#include "utils.h"
#include "libevcs.h"

#include <time.h>

extern struct rr_cfg_file g_rr_cfg_file;

static void init_IMEI_data(gps_info_t *gps_info, road_info_t *road_info, KV_IMEI *kv_IMEI)
{
	kv_IMEI->count = 1;
	kv_IMEI->rt = road_info->rt;
	kv_IMEI->max_speed_num = gps_info->max_speed;
	kv_IMEI->max_speed = gps_info->max_speed;
	kv_IMEI->str_time = gps_info->start_time;
	kv_IMEI->end_time = gps_info->end_time;
	kv_IMEI->roadID = road_info->new_roadID;
	strcpy(kv_IMEI->IMEI, gps_info->IMEI);
	kv_IMEI->citycode = road_info->city_code;
	kv_IMEI->countycode = road_info->county_code;
	kv_IMEI->direction = gps_info->direction;
	kv_IMEI->start_lon = road_info->start_lon;
	kv_IMEI->start_lat = road_info->start_lat;
	kv_IMEI->end_lon = road_info->end_lon;
	kv_IMEI->end_lat = road_info->end_lat;
}

static int add_redis_task(char command[], struct rr_link *link, struct ev_loop *loop)
{
	struct xpool            *cpool = conn_xpool_find(link->host, link->port);
	struct async_api        *api = async_api_initial(loop, 1, true, QUEUE_TYPE_FIFO, NEXUS_TYPE_TEAM,
			NULL, NULL, NULL);

	// x_printf(D,"import_to_redis: %s\n",command);

	if (api && cpool) {
		/*data*/
		char    *proto;
		int     ok = cmd_to_proto(&proto, command);

		if (ok == REDIS_ERR) {
			if (proto) {
				free(proto);
			}

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

static int update_redis(SECKV_ROAD *kv_roadID, int limit, struct ev_loop *loop)
{
	int     old_rr_id = kv_roadID->old_roadID / 1000;
	int     old_sg_id = kv_roadID->old_roadID % 1000;
	int     save_time = g_rr_cfg_file.save_time;
	char    section[204] = "";

	single_road_section(section, kv_roadID);

	/*添加单条道路路况*/
	char redis_buff[BUFF_USE_LEN] = { 0 };
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"SET %ld:roadSpeedInfo %d@%d@%ld@%ld@%s",
		kv_roadID->old_roadID, kv_roadID->max_speed, kv_roadID->avg_speed,
		kv_roadID->end_time, kv_roadID->used_time, kv_roadID->IMEI);
	x_printf(D, "redis command: %s", redis_buff);

	add_redis_task(redis_buff, &(g_rr_cfg_file.road_traffic_server), loop);

#if 0//FIXME:baoxue
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"hset %d:cityinfo %ld %ld@%d:%d@%s",
		kv_roadID->citycode, kv_roadID->old_roadID, kv_roadID->end_time, old_rr_id, old_sg_id, section);
#else
	/*删除过期城市道路路况*/
	time_t now_time;
	time(&now_time);
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"ZREMRANGEBYSCORE %d:cityinfo -inf %ld",
		kv_roadID->citycode, now_time - save_time);
	add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), loop);

	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"ZADD %d:cityinfo %ld %d:%d:%d:%d:%ld:%ld",
		kv_roadID->citycode, kv_roadID->end_time, old_rr_id, old_sg_id, kv_roadID->max_speed,
		kv_roadID->avg_speed, kv_roadID->used_time, kv_roadID->end_time);
#endif
	x_printf(D, "redis command: %s", redis_buff);
	add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), loop);

	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"hset %d:countyinfo %ld %ld@%d:%d@%s",
		kv_roadID->countycode, kv_roadID->old_roadID, kv_roadID->end_time, old_rr_id, old_sg_id, section);
	x_printf(D, "redis command: %s", redis_buff);
	add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), loop);

	return 0;
}

static int set_IMEI_data(gps_info_t *gps_info, road_info_t *road_info, KV_IMEI *kv_IMEI)
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

static void set_roadID_data(KV_IMEI *kv_IMEI, SECKV_ROAD *kv_roadID)
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

	// kv_roadID->road_sec[0].endtime = gps_info->end_time;
	kv_roadID->road_sec[0].avg_speed = kv_roadID->avg_speed;
	kv_roadID->road_sec[0].longitude = kv_IMEI->start_lon;
	kv_roadID->road_sec[0].latitude = kv_IMEI->start_lat;

	kv_roadID->road_sec[1].avg_speed = kv_roadID->avg_speed;
	kv_roadID->road_sec[1].longitude = kv_IMEI->end_lon;
	kv_roadID->road_sec[1].latitude = kv_IMEI->end_lat;
	kv_roadID->road_sec[1].endtime = kv_IMEI->end_time;
}

static int single_new_road(struct ev_loop *loop, KV_IMEI kv_IMEI, SECKV_ROAD kv_roadID, gps_info_t *gps_info, road_info_t *road_info, struct single_model_t *single)
{
	if (kv_IMEI.count >= single->single_cfg.road_match_limit) {
		set_roadID_data(&kv_IMEI, &kv_roadID);

		if (ERR_IMEI == set_roadID_to_kv(&kv_roadID)) {
			x_printf(D, "***set_roadID_to_kv failed***\n");
			return ERR_IMEI;
		}

		x_printf(D,
			"kv_roadID IMEI %s max_speed %d avg_speed %d "
			"end_time %ld used_time %ld old_roadID %ld city_code %d "
			"county_code %d",
			kv_roadID.IMEI, kv_roadID.max_speed,
			kv_roadID.avg_speed, kv_roadID.end_time,
			kv_roadID.used_time, kv_roadID.old_roadID,
			kv_roadID.citycode, kv_roadID.countycode);

		single->single_update(&kv_roadID, single->single_cfg.expire_time, loop);
	}

	init_IMEI_data(gps_info, road_info, &kv_IMEI);
	x_printf(D, "set kv_IMEI.roadID %ld\n", kv_IMEI.roadID);

	if (ERR_IMEI == set_IMEI_to_kv(&kv_IMEI)) {
		x_printf(D, "***set_IMEI_to_kv failed***\n");
		return ERR_IMEI;
	}

	return SUC_IMEI;
}

int single_new_road_2(struct ev_loop *loop, KV_IMEI kv_IMEI, SECKV_ROAD kv_roadID, gps_info_t *gps_info, road_info_t *road_info, int limit)
{
	if (kv_IMEI.count >= limit) {
		set_roadID_data(&kv_IMEI, &kv_roadID);

		if (ERR_IMEI == set_roadID_to_kv(&kv_roadID)) {
			x_printf(D, "***set_roadID_to_kv failed***\n");
			return ERR_IMEI;
		}

		x_printf(D,
			"kv_roadID IMEI %s max_speed %d avg_speed %d "
			"end_time %ld used_time %ld old_roadID %ld city_code %d "
			"county_code %d",
			kv_roadID.IMEI, kv_roadID.max_speed,
			kv_roadID.avg_speed, kv_roadID.end_time,
			kv_roadID.used_time, kv_roadID.old_roadID,
			kv_roadID.citycode, kv_roadID.countycode);

		update_redis(&kv_roadID, 600, loop);
	}

	init_IMEI_data(gps_info, road_info, &kv_IMEI);
	x_printf(D, "set kv_IMEI.roadID %ld\n", kv_IMEI.roadID);

	if (ERR_IMEI == set_IMEI_to_kv(&kv_IMEI)) {
		x_printf(D, "***set_IMEI_to_kv failed***\n");
		return ERR_IMEI;
	}

	return SUC_IMEI;
}

static int single_same_road(gps_info_t *gps_info, road_info_t *road_info, KV_IMEI kv_IMEI)
{
	if (ERR_IMEI == set_IMEI_data(gps_info, road_info, &kv_IMEI)) {	// same road
		x_printf(E, "***gps_info package sequence error***\n");
		return ERR_IMEI;
	}

	x_printf(D, "set kv_IMEI.roadID %ld\n", kv_IMEI.roadID);

	if (ERR_IMEI == set_IMEI_to_kv(&kv_IMEI)) {
		x_printf(D, "***set_IMEI_to_kv failed***\n");
		return ERR_IMEI;
	}

	return SUC_IMEI;
}

int single_calculate(struct ev_loop *p_loop, gps_info_t *gps_info, road_info_t *road_info, struct single_model_t *single)
{
	KV_IMEI         kv_IMEI = { 0 };
	SECKV_ROAD      kv_roadID = { 0 };
	int             ok = get_IMEI_from_kv(gps_info->IMEI, &kv_IMEI);// get redis_IMEI data based on IMEI

	x_printf(D, "get kv_IMEI.roadID %ld\n", kv_IMEI.roadID);
	x_printf(D, "calculate gps endtime:%ld\n", gps_info->end_time);

	switch (ok)
	{
		case ERR_IMEI:
			return ERR_IMEI;

		case SUC_IMEI:

			if (kv_IMEI.roadID != road_info->new_roadID) {	// change road
				if (ERR_IMEI == single_new_road(p_loop, kv_IMEI, kv_roadID, gps_info, road_info, single)) {
					return ERR_IMEI;
				}
			} else {
				if (ERR_IMEI == single_same_road(gps_info, road_info, kv_IMEI)) {
					return ERR_IMEI;
				}
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

