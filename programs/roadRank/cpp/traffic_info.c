#include "traffic_info.h"
#include "rr_cfg.h"
#include "pool_api.h"
#include "async_api.h"
#include "utils.h"

#include <time.h>

#define ASYNC_LIBEV_THRESHOLD   6
#define ASYNC_LUAKV_THRESHOLD   1
#define BUFF_USE_LEN            10240

extern struct rr_cfg_file g_rr_cfg_file;

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

int single_update_redis(SECKV_ROAD *kv_roadID, int g_save_time, struct ev_loop *loop)
{
	struct async_ctx *ac = NULL;

	ac = async_initial(loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, ASYNC_LIBEV_THRESHOLD);

	if (!ac) {
		return 0;
	}

	int     old_rr_id = kv_roadID->old_roadID / 1000;
	int     old_sg_id = kv_roadID->old_roadID % 1000;
	int     save_time = g_rr_cfg_file.save_time;

	char section[204] = "";
	single_road_section(section, kv_roadID);

	/*添加单条道路路况*/
	char redis_buff[BUFF_USE_LEN] = { 0 };
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"SET %ld:roadSpeedInfo %d@%d@%ld@%ld@%lld",
		kv_roadID->old_roadID, kv_roadID->max_speed, kv_roadID->avg_speed,
		kv_roadID->end_time, kv_roadID->used_time, kv_roadID->IMEI);
	x_printf(D, "redis command: %s", redis_buff);

	add_redis_task(redis_buff, &(g_rr_cfg_file.road_traffic_server), ac);
#if 0
	/*删除过期城市道路路况*/
	time_t now_time;
	time(&now_time);
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"ZREMRANGEBYSCORE %d:cityinfo -inf %ld",
		kv_roadID->citycode, now_time - save_time);
	add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), ac);

	/*添加城市道路路况*/
	memset(redis_buff, 0, BUFF_USE_LEN);

	/*sprintf(redis_buff,
	 *                "ZADD %d:cityinfo %ld %d:%d:%d:%d:%ld:%ld",
	 *                kv_roadID->citycode, kv_roadID->end_time, old_rr_id, old_sg_id, kv_roadID->max_speed,
	 *                kv_roadID->avg_speed, kv_roadID->used_time, kv_roadID->end_time);*/
	sprintf(redis_buff,
		"ZADD %d:cityinfo %ld %d:%d@%s",
		kv_roadID->citycode, kv_roadID->end_time, old_rr_id, old_sg_id, section);
	x_printf(D, "redis command: %s", redis_buff);
	add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), ac);

	/*删除过期区县道路路况*/
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"ZREMRANGEBYSCORE %d:countyinfo -inf %ld",
		kv_roadID->countycode, now_time - save_time);
	add_redis_task(redis_buff, &(g_rr_cfg_file.county_traffic_server), ac);

	/*添加城市道路路况*/
	memset(redis_buff, 0, BUFF_USE_LEN);

	/*sprintf(redis_buff,
	 *                "ZADD %d:countyinfo %ld %d:%d:%d:%d:%ld:%ld",
	 *                kv_roadID->countycode, kv_roadID->end_time, old_rr_id, old_sg_id, kv_roadID->max_speed,
	 *                kv_roadID->avg_speed, kv_roadID->used_time, kv_roadID->end_time);*/
	sprintf(redis_buff,
		"ZADD %d:countyinfo %ld %d:%d@%s",
		kv_roadID->countycode, kv_roadID->end_time, old_rr_id, old_sg_id, section);
	x_printf(D, "redis command: %s", redis_buff);
	add_redis_task(redis_buff, &(g_rr_cfg_file.county_traffic_server), ac);
#endif /* if 0 */
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"hset %d:cityinfo %ld %ld@%d:%d@%s",
		kv_roadID->citycode, kv_roadID->old_roadID, kv_roadID->end_time, old_rr_id, old_sg_id, section);
	x_printf(D, "redis command: %s", redis_buff);
	add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), ac);

	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"hset %d:countyinfo %ld %ld@%d:%d@%s",
		kv_roadID->countycode, kv_roadID->old_roadID, kv_roadID->end_time, old_rr_id, old_sg_id, section);
	x_printf(D, "redis command: %s", redis_buff);
	add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), ac);
	async_startup(ac);

	return 0;
}

int subsec_update_redis(SECKV_ROAD *kv_roadID, int g_save_time, struct ev_loop *loop)
{
	struct async_ctx *ac = NULL;

	ac = async_initial(loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, ASYNC_LIBEV_THRESHOLD);

	if (!ac) {
		return 0;
	}

	int     old_rr_id = kv_roadID->old_roadID / 1000;
	int     old_sg_id = kv_roadID->old_roadID % 1000;
	int     save_time = g_rr_cfg_file.save_time;
	char    section[4096] = "";
	show_road_section(section, kv_roadID);

	// 添加单条道路路况
	char redis_buff[BUFF_USE_LEN] = { 0 };
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"SET %ld:roadSpeedInfo %s@%d@%d@%ld@%ld@%lld",
		kv_roadID->old_roadID, section, kv_roadID->max_speed, kv_roadID->avg_speed,
		kv_roadID->end_time, kv_roadID->used_time, kv_roadID->IMEI);
	x_printf(D, "subsection redis command: %s", redis_buff);

	add_redis_task(redis_buff, &(g_rr_cfg_file.road_traffic_server), ac);

	/*
	 *       //删除过期城市道路路况
	 *       time_t now_time;
	 *       time(&now_time);
	 *       memset(redis_buff, 0, BUFF_USE_LEN);
	 *       sprintf(redis_buff,
	 *                       "ZREMRANGEBYSCORE %d:cityinfo -inf %ld",
	 *                       kv_roadID->citycode, now_time - save_time);
	 *       add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), ac);
	 *
	 *       //添加城市道路路况
	 *       memset(redis_buff, 0, BUFF_USE_LEN);
	 *       sprintf(redis_buff,
	 *                       "ZADD %d:cityinfo %ld %d:%d@%s",
	 *                       kv_roadID->citycode, kv_roadID->end_time, old_rr_id, old_sg_id, section);
	 *       x_printf(D, "redis command: %s", redis_buff);
	 *       add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), ac);
	 */
	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"hset %d:cityinfo %ld %ld@%d:%d@%s",
		kv_roadID->citycode, kv_roadID->old_roadID, kv_roadID->end_time, old_rr_id, old_sg_id, section);
	x_printf(D, "redis command: %s", redis_buff);
	add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), ac);

	/*
	 *   //删除过期区县道路路况
	 *   memset(redis_buff, 0, BUFF_USE_LEN);
	 *   sprintf(redis_buff,
	 *                "ZREMRANGEBYSCORE %d:countyinfo -inf %ld",
	 *                kv_roadID->countycode, now_time - save_time);
	 *   add_redis_task(redis_buff, &(g_rr_cfg_file.county_traffic_server), ac);
	 *
	 *   //添加城市道路路况
	 *   memset(redis_buff, 0, BUFF_USE_LEN);
	 *   sprintf(redis_buff,
	 *                "ZADD %d:countyinfo %ld %d:%d@%s",
	 *                kv_roadID->countycode, kv_roadID->end_time, old_rr_id, old_sg_id, section);
	 *   x_printf(D, "redis command: %s", redis_buff);
	 *   add_redis_task(redis_buff, &(g_rr_cfg_file.county_traffic_server), ac);
	 */

	memset(redis_buff, 0, BUFF_USE_LEN);
	sprintf(redis_buff,
		"hset %d:countyinfo %ld %ld@%d:%d@%s",
		kv_roadID->countycode, kv_roadID->old_roadID, kv_roadID->end_time, old_rr_id, old_sg_id, section);
	x_printf(D, "redis command: %s", redis_buff);
	add_redis_task(redis_buff, &(g_rr_cfg_file.city_traffic_server), ac);
	async_startup(ac);

	return 0;
}

