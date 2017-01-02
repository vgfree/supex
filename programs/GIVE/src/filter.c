/**
 * Author       : chenzutao
 * Date         : 2015-10-27
 * Function     : filter GPS data
 **/

#include "filter.h"

// #define CITY_COUNT 2048
extern struct rr_cfg_file g_rr_cfg_file;

// int g_city_code[CITY_COUNT];

int citycode_filter(kv_handler_t *hid, double lon, double lat)
{
	char cmd[64] = { 0 };

	// sprintf(cmd, "HGET %d&%d cityCode", (int)(lon*100), (int)(lat*100));
	sprintf(cmd, "GET %d&%d", (int)(lon * 100), (int)(lat * 100));

	kv_handler_t *handler = kv_spl(hid, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return GV_ERR;
        }

	int             citycode = 0;
	unsigned long           len = answer_length(ans);

	if (len == 1) {
		kv_answer_value_t *value = answer_head_value(ans);
		if (answer_value_look_type(value) == VALUE_TYPE_NIL) {
			kv_handler_release(handler);
			x_printf(W, "can't find citycode lon:%ld lat:%ld\n", lon, lat);
			return 10000;
		}

		char ptr[128] = {0};
		memcpy(ptr, answer_value_look_addr(value), answer_value_look_size(value));
		citycode = atoi((char *)ptr);
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

int type_add(kv_handler_t *hid, long long IMEI, int type)
{
	char cmd[32] = { 0 };

	sprintf(cmd, "HSET %lld type %d", IMEI, type);
	kv_handler_t *handler = kv_spl(hid, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }
	kv_handler_release(handler);

	sprintf(cmd, "HGET %lld type", IMEI);
	handler = kv_spl(hid, cmd, strlen(cmd));
	ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }
	kv_answer_value_t *value = answer_head_value(ans);
	if (answer_value_look_type(value) == VALUE_TYPE_STAR) {
		char ptr[128] = {0};
		memcpy(ptr, answer_value_look_addr(value), answer_value_look_size(value));
		
		if ((atoi(ptr) == type)) {
			x_printf(E, "[%s] executed succeed.\n", cmd);
			kv_handler_release(handler);
			return 0;
		}
	}

	kv_handler_release(handler);
	return -1;
}

int status_add(kv_handler_t *hid, long long IMEI, int status)
{
	char cmd[32] = { 0 };

	sprintf(cmd, "HSET %lld status %d", IMEI, status);
	kv_handler_t *handler = kv_spl(hid, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }
	kv_handler_release(handler);

	sprintf(cmd, "HGET %lld status", IMEI);
	handler = kv_spl(hid, cmd, strlen(cmd));
	ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }
	kv_answer_value_t *value = answer_head_value(ans);
	if (answer_value_look_type(value) == VALUE_TYPE_STAR) {
		char ptr[128] = {0};
		memcpy(ptr, answer_value_look_addr(value), answer_value_look_size(value));
		
		if ((atoi(ptr) == status)) {
			x_printf(E, "[%s] executed succeed.\n", cmd);
			kv_handler_release(handler);
			return 0;
		}
	}

	kv_handler_release(handler);
	return -1;
}

int type_filter(kv_handler_t *hid, long long IMEI, int type)
{
	char cmd[32] = { 0 };

	sprintf(cmd, "HMGET %lld type", IMEI);
	kv_handler_t *handler = kv_spl(hid, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }
	kv_answer_value_t *value = answer_head_value(ans);
	if (answer_value_look_type(value) == VALUE_TYPE_STAR) {
		char ptr[128] = {0};
		memcpy(ptr, answer_value_look_addr(value), answer_value_look_size(value));
		
		if ((atoi(ptr) == type)) {
			kv_handler_release(handler);
			return 0;
		}
	}

	kv_handler_release(handler);
	return -1;
}

int status_filter(kv_handler_t *hid, long long IMEI, int status)
{
	char cmd[32] = { 0 };

	sprintf(cmd, "HMGET %lld status", IMEI);

	kv_handler_t *handler = kv_spl(hid, cmd, strlen(cmd));
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }
	kv_answer_value_t *value = answer_head_value(ans);
	if (answer_value_look_type(value) == VALUE_TYPE_STAR) {
		char ptr[128] = {0};
		memcpy(ptr, answer_value_look_addr(value), answer_value_look_size(value));
		
		if ((atoi(ptr) == status)) {
			kv_handler_release(handler);
			return 0;
		}
	}

	kv_handler_release(handler);
	return -1;
}

/*
 *   int filter(long long IMEI, double lon, double lat, char *model)
 *   {
 *   }
 */

