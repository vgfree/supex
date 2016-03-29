#include "decode_gps.h"
#include "rr_cfg.h"
#include "forward_imei.h"
extern struct rr_cfg_file g_rr_cfg_file;

int gps_decode(struct ev_loop *loop, const char *data, GPS_INFO *gps_info)
{
    int     i;
	cJSON   *arr = NULL;
	cJSON   *son = NULL;
	cJSON   *obj = cJSON_Parse(data);

	if (NULL == obj) {
		x_printf(E, "Failed to create cJSON object!");
		return -1;
	}

	/*parse IMEI*/
	son = cJSON_GetObjectItem(obj, "IMEI");

	if (NULL == son) {
		x_printf(E, "data has no IMEI!");
		goto fail;
	}

	gps_info->IMEI = atoll(son->valuestring);

    /*检查imei是否是测试imei*/
    if(forward_imei_check(gps_info->IMEI)) {
        forward_data(data, loop, g_rr_cfg_file.forward_server.host, g_rr_cfg_file.forward_server.port);
        cJSON_Delete(obj);
        return -1;
    }

	/*parse speed*/
	son = cJSON_GetObjectItem(obj, "speed");

	if (NULL == son) {
		x_printf(E, "data has no speed!");
		goto fail;
	}

	int all = cJSON_GetArraySize(son);

	if ((all <= 0) || (all > 15)) {
		x_printf(E, "Failed to decode GPS, the array number of speed is not right!\n");
		goto fail;
	}

	//	all = MIN(all, 5); //TODO
	gps_info->point_cnt = all;
	arr = cJSON_GetArrayItem(son, 0);
	int     speed = arr->valueint;
	int     speed_max_idx = 0;
	gps_info->max_speed = speed;

	for (i = 0; i < all; i++) {
		arr = cJSON_GetArrayItem(son, i);
		speed = arr->valueint;

		if (speed > gps_info->max_speed) {
			speed_max_idx = i;
			gps_info->max_speed = speed;
		}

		x_printf(D, "\t%d\n", speed);
	}

	x_printf(D, "max speed\t%d\n", gps_info->max_speed);

	if (gps_info->max_speed < g_rr_cfg_file.min_speed_limit) {
        x_printf(I, "max speed < %d!, %s", g_rr_cfg_file.min_speed_limit, data);
		goto fail;
	}

	/*parse direction*/
	son = cJSON_GetObjectItem(obj, "direction");

	if (NULL == son) {
		x_printf(E, "data has no direction!");
		goto fail;
	}

	arr = cJSON_GetArrayItem(son, speed_max_idx);
	int direction = arr->valueint;

	if (direction == -1) {
		x_printf(E, "data error, direction is -1");
		goto fail;
	}

	gps_info->direction = direction;

	/*parse longitude*/
	son = cJSON_GetObjectItem(obj, "longitude");

	if (NULL == son) {
		x_printf(E, "data has no longitude!");
		goto fail;
	}

	arr = cJSON_GetArrayItem(son, speed_max_idx);
	gps_info->longitude = arr->valuedouble;

	/*parse latitude*/
	son = cJSON_GetObjectItem(obj, "latitude");

	if (NULL == son) {
		x_printf(E, "data has no latitude!");
		goto fail;
	}

	arr = cJSON_GetArrayItem(son, speed_max_idx);
	gps_info->latitude = arr->valuedouble;

	/*parse GPSTime*/
	son = cJSON_GetObjectItem(obj, "GPSTime");

	if (NULL == son) {
		x_printf(E, "data has no GPSTime!");
		goto fail;
	}

	long    gps_time1 = 0;
	long    gps_time2 = 0;

	arr = cJSON_GetArrayItem(son, 0);
	gps_time1 = arr->valueint;
	arr = cJSON_GetArrayItem(son, gps_info->point_cnt - 1);
	gps_time2 = arr->valueint;

	if (gps_time1 > gps_time2) {
		gps_info->start_time = gps_time2;
		gps_info->end_time = gps_time1;
	} else {
		gps_info->start_time = gps_time1;
		gps_info->end_time = gps_time2;
	}

	cJSON_Delete(obj);

	x_printf(D, "<=====================================================================>");
    x_printf(D, "direction:%d, longitude:%f, latitude:%f, start_time:%ld, end_time:%ld, IMEI:%lld",
		gps_info->direction, gps_info->longitude, gps_info->latitude,
		gps_info->start_time, gps_info->end_time, gps_info->IMEI);
	return 0;

fail:
	cJSON_Delete(obj);
	return -1;
}

