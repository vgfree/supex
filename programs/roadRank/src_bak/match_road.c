#include "match_road.h"
#include "calculate.h"

#define ASYNC_LIBEV_THRESHOLD 10

extern struct rr_cfg_file g_rr_cfg_file;

void PMR_callback(struct async_ctx *ac, void *reply, void *data)
{
	CAL_INFO                *cal_info = data;
	struct redis_reply      *p_reply = reply;

	x_printf(D, "------------\n");

	//	x_printf(D, "IMEI IS:[%ld]\tPMR_callback data: %s", cal_info->gps_info->IMEI,
	//		ac->obj.replies.work->cache.buf_addr);
	if (p_reply && (p_reply->elements != 0)) {
		ROAD_INFO road_info = { 0 };

		if (REDIS_REPLY_ARRAY == p_reply->type) {
			road_info.road_rootID = atoll(p_reply->element[0]->str);
			road_info.segmentID = atoll(p_reply->element[1]->str);
			road_info.countycode = atoll(p_reply->element[3]->str);
			road_info.rt = atoi(p_reply->element[4]->str);

			road_info.citycode = (road_info.countycode / 100) * 100;
			road_info.new_roadID = road_info.road_rootID * 1000 + road_info.segmentID;

			x_printf(D,
                "road_info road_rootID %ld segmentID %d countycode %d rt %d citycode %d new_roadID %ld\n",
				road_info.road_rootID, road_info.segmentID, road_info.countycode, road_info.rt,
				road_info.citycode, road_info.new_roadID);
			struct ev_loop *loop = ac->obj.settings.loop;

			cal_info->cal_callback(&(cal_info->gps_info), (void *)&road_info, loop);
		}
	}

	free(data);
}

static void pmr_abnormal_cb(const struct async_obj *obj, void *data)
{
	struct command_node *p_work = obj->replies.work;

	if (p_work->privdata) {
		free(p_work->privdata);
		p_work->privdata = NULL;
	}
}

static int forward_to_server(char *host, int port, const char *data, size_t size, struct ev_loop *loop, ASYNC_CALL_BACK fncb, void *with)
{
	struct cnt_pool         *cpool = NULL;
	struct async_ctx        *ac = NULL;

	ac = async_initial(loop, QUEUE_TYPE_FIFO, pmr_abnormal_cb, NULL, NULL, 1);

	if (ac) {
		void    *sfd = (void *)(intptr_t)-1;
		int     rc = conn_xpool_gain(&cpool, host, port, &sfd);

		if (rc) {
			async_distory(ac);
			return -1;
		}

		/*send*/
		async_command(ac, PROTO_TYPE_REDIS, (int)(intptr_t)sfd, cpool, fncb, with, data, size);

		async_startup(ac);
		return 0;
	}

	return -1;
}

int match_road(struct ev_loop *loop, CAL_INFO *cal_info)
{
    char    *host = g_rr_cfg_file.pmr_server.host;
    int     port = g_rr_cfg_file.pmr_server.port;
	int     ok = -1;
	char    buff[128] = { 0 };

	char *proto = NULL;

	sprintf(buff, "hmget LOCATE %lf %lf %d", cal_info->gps_info.longitude,
		cal_info->gps_info.latitude, cal_info->gps_info.direction);
	x_printf(D, "redis command: %s", buff);
	cmd_to_proto(&proto, buff);

	if (proto) {
		ok = forward_to_server(host, port, proto, strlen(proto), loop, PMR_callback, cal_info);
		free(proto);
	}

	return ok;
}

