/**
 * Author       : chenzutao
 * Date         : 2015-10-27
 * Function     : filter GPS data
 **/

#include "filter.h"

// #define CITY_COUNT 2048
extern struct rr_cfg_file g_rr_cfg_file;

// int g_city_code[CITY_COUNT];

int citycode_filter(kv_handler_t *handler, double lon, double lat)
{
	char cmd[64] = { 0 };

	// sprintf(cmd, "HGET %d&%d cityCode", (int)(lon*100), (int)(lat*100));
	sprintf(cmd, "GET %d&%d", (int)(lon * 100), (int)(lat * 100));

	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans->errnum != ERR_NONE) {
                if (ans->errnum == ERR_NIL) {
                        kv_answer_release(ans);
                        x_printf(W, "can't find citycode errnum:%d, errstr:%s\n", ans->errnum, ans->err);
                        return 10000;
                }
		kv_answer_release(ans);
		x_printf(E, "errnum:%d, errstr:%s\n", ans->errnum, ans->err);
		return GV_ERR;
	}

	int             citycode = 0;
	unsigned long   len = kv_answer_length(ans);

	if (len == 1) {
		kv_answer_value_t *value = kv_answer_first_value(ans);

		if (value) {
			citycode = atoi((char *)value->ptr);
		} else {
			x_printf(E, "Failed to get cityCode by:%s\n", cmd);
			kv_answer_release(ans);
			return GV_ERR;
		}
	}

	x_printf(D, "cityCode:%d, lon&lat:[%.7lf, %.7lf]\n", citycode, lon, lat);

	/*
	 *   int left = 0;
	 *   int right = g_rr_cfg_file.city_size -1;
	 *   int mid;
	 *   while(left <= right && left >= 0 && right<g_rr_cfg_file.city_size)
	 *   {
	 *        mid = (left + right) >> 1;
	 *        if(g_rr_cfg_file.citycode[mid] == citycode)
	 *        {
	 *                x_printf(I, "cityCode:%d, lon&lat:[%.7lf, %.7lf]\n", citycode, lon, lat);
	 *                kv_answer_release(ans);
	 *                return 0;
	 *        }
	 *        if(g_rr_cfg_file.citycode[mid] < citycode)
	 *        {
	 *                left = mid + 1;
	 *        }
	 *        else if(g_rr_cfg_file.citycode[mid] > citycode)
	 *        {
	 *                right = mid -1;
	 *        }
	 *   }
	 */
	kv_answer_release(ans);
	return citycode;
}

int type_add(kv_handler_t *handler, long long IMEI, int type)
{
	char cmd[32] = { 0 };

	sprintf(cmd, "HSET %lld type %d", IMEI, type);
	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans && (ans->errnum != ERR_NONE)) {
		x_printf(E, "errnum:%d, errstr:%s\n", ans->errnum, ans->err);
		return -1;
	}

	kv_answer_value_t *value = kv_answer_first_value(ans);
	// x_printf(E, "HSET type: %s\n", (char *)value->ptr);
	kv_answer_release(ans);

	sprintf(cmd, "HGET %lld type", IMEI);
	ans = kv_ask(handler, cmd, strlen(cmd));
	value = kv_answer_first_value(ans);

	// x_printf(E, "HGET type: %s\n", (char *)value->ptr);
	if (value && (atoi((char *)value->ptr) == type)) {
		x_printf(E, "[%s] executed succeed.\n", cmd);
		kv_answer_release(ans);
		return 0;
	}

	kv_answer_release(ans);
	return -1;
}

int status_add(kv_handler_t *handler, long long IMEI, int status)
{
	char cmd[32] = { 0 };

	sprintf(cmd, "HSET %lld status %d", IMEI, status);
	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans && (ans->errnum != ERR_NONE)) {
		x_printf(E, "errnum:%d, errstr:%s\n", ans->errnum, ans->err);
		return -1;
	}

	kv_answer_release(ans);

	sprintf(cmd, "HGET %lld status", IMEI);
	ans = kv_ask(handler, cmd, strlen(cmd));
	kv_answer_value_t *value = kv_answer_first_value(ans);

	// x_printf(E, "HGET status : %s\n", (char *)value->ptr);
	if (value && (atoi((char *)value->ptr) == status)) {
		x_printf(E, "[%s] executed succeed.\n", cmd);
		kv_answer_release(ans);
		return 0;
	}

	kv_answer_release(ans);
	return -1;
}

int type_filter(kv_handler_t *handler, long long IMEI, int type)
{
	char cmd[32] = { 0 };

	sprintf(cmd, "HMGET %lld type", IMEI);
	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans && (ans->errnum != ERR_NONE)) {
		x_printf(E, "errnum:%d, errstr:%s\n", ans->errnum, ans->err);
		return -1;
	}

	kv_answer_value_t *value = kv_answer_first_value(ans);

	if (value && (atoi(value->ptr) == type)) {
		kv_answer_release(ans);
		return 0;
	}

	kv_answer_release(ans);
	return -1;
}

int status_filter(kv_handler_t *handler, long long IMEI, int status)
{
	char cmd[32] = { 0 };

	sprintf(cmd, "HMGET %lld status", IMEI);
	kv_answer_t *ans = kv_ask(handler, cmd, strlen(cmd));

	if (ans && (ans->errnum != ERR_NONE)) {
		x_printf(E, "errnum:%d, errstr:%s\n", ans->errnum, ans->err);
		return -1;
	}

	kv_answer_value_t *value = kv_answer_first_value(ans);

	if (value && (atoi(value->ptr) == status)) {
		kv_answer_release(ans);
		return 0;
	}

	kv_answer_release(ans);
	return -1;
}

/*
 *   int filter(long long IMEI, double lon, double lat, char *model)
 *   {
 *   }
 */

