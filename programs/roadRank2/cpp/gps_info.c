#include "gps_info.h"
#include "forward_imei.h"
#include "kv_speed.h"

#include <string.h>
#include <stdlib.h>
#include "cJSON.h"

int gps_decode(struct ev_loop *loop, const char *p_data, gps_info_t *p_gps, rr_link forward_link)
{
	int     i = 0;
	int     all = 0;
	int     all_speed = 0;
	cJSON   *arr_speed = NULL;
	cJSON   *son = NULL;
	cJSON   *son_speed = NULL;
	cJSON   *obj = cJSON_Parse(p_data);

	if (NULL == obj) {
		x_printf(E, "Failed to create cJSON object!");
		return -1;
	}

	/*parse IMEI*/
	son = cJSON_GetObjectItem(obj, "IMEI");

	if (son == NULL) {
		son = cJSON_GetObjectItem(obj, "mirrtalkID");
	}

	if (NULL == son) {
		x_printf(E, "data has no IMEI!");
		goto fail;
	}

	strncpy(p_gps->IMEI, son->valuestring, strlen(son->valuestring));
	x_printf(D, "[gps_decode]::IMEI:%s\n", p_gps->IMEI);

	/*检查imei是否是测试imei*/
	if (forward_imei_check(p_gps->IMEI)) {
		forward_data(p_data, loop, forward_link.host, forward_link.port);
		cJSON_Delete(obj);
		return -2;
	}

	/*parse tokenCode */
	son = cJSON_GetObjectItem(obj, "tokenCode");

	if (NULL == son) {
		x_printf(E, "data has no tokenCode!");
		goto fail;
	}

	strncpy(p_gps->tokenCode, son->valuestring, strlen(son->valuestring));
	x_printf(D, "[gps_decode]::tokenCode:%s\n", p_gps->tokenCode);
	/*parse speed*/
	son_speed = cJSON_GetObjectItem(obj, "speed");

	if (NULL == son_speed) {
		x_printf(E, "data has no speed!");
		goto fail;
	}

	all_speed = cJSON_GetArraySize(son_speed);

	if ((all_speed <= 0) || (all_speed > 15)) {
		x_printf(E, "Failed to decode GPS, the array number of speed is not right!\n");
		goto fail;
	}

	// 读取libkv
	kv_speed kv_speed_ex = { .tokenCode = { '\0' }, .sign = 0 };
	get_speed_from_kv(p_gps->IMEI, &kv_speed_ex);
	x_printf(D, "kv_speed_ex.tokenCode:%s,len:%ld,kv_speed_ex.sign:%d,\n", kv_speed_ex.tokenCode, strlen(kv_speed_ex.tokenCode), kv_speed_ex.sign);
#if 1
	if (strcmp(p_gps->tokenCode, kv_speed_ex.tokenCode) == 0) {
		// printf("equal --- \n");
		int flag = 0;

		for (i = 0; i < all_speed; i++) {
			arr_speed = cJSON_GetArrayItem(son_speed, i);
			int speed = arr_speed->valueint;

			if (kv_speed_ex.sign == 1) {
				// 只要查找第一个speed >0
				if ((speed > 0) && (flag == 0)) {
					kv_speed_ex.sign = 2;
					strncpy(kv_speed_ex.tokenCode, p_gps->tokenCode, strlen(p_gps->tokenCode));
					set_speed_to_kv(p_gps->IMEI, &kv_speed_ex);
					flag = 1;
					// printf("### sign 2\n");
				}
			} else if (kv_speed_ex.sign == 0) {
				if ((speed == 0) && (flag == 0)) {
					kv_speed_ex.sign = 1;
					strncpy(kv_speed_ex.tokenCode, p_gps->tokenCode, strlen(p_gps->tokenCode));
					set_speed_to_kv(p_gps->IMEI, &kv_speed_ex);
					flag = 1;
				} else {
					if (flag == 1) {
						kv_speed_ex.sign = 2;
						strncpy(kv_speed_ex.tokenCode, p_gps->tokenCode, strlen(p_gps->tokenCode));
						set_speed_to_kv(p_gps->IMEI, &kv_speed_ex);
						flag = 2;
					}
				}
			} else if (kv_speed_ex.sign == 2) {
				break;
			}
		}

		if (flag > 0) {
			cJSON_Delete(obj);
			return -3;
		}
	} else {
		// printf("no equal --- \n");
		int     flag = all_speed;
		int     flag_se = 0;

		// 新的tokenCode
		for (i = 0; i < all_speed; i++) {
			arr_speed = cJSON_GetArrayItem(son_speed, i);
			int speed = arr_speed->valueint;

			// printf("speed:%d\n",speed);
			// if (kv_speed_ex.sign == 0) {
			// first stop start
			if ((speed == 0) && (flag_se == 0)) {
				kv_speed_ex.sign = 1;
				strncpy(kv_speed_ex.tokenCode, p_gps->tokenCode, strlen(p_gps->tokenCode));
				set_speed_to_kv(p_gps->IMEI, &kv_speed_ex);
				flag = i;
				// printf("0 first \n");
			} else {
				// printf("i:%d,flag:%d\n",i,flag);
				// first stop end
				if ((flag < i) && (flag_se == 0)) {
					kv_speed_ex.sign = 2;
					strncpy(kv_speed_ex.tokenCode, p_gps->tokenCode, strlen(p_gps->tokenCode));
					set_speed_to_kv(p_gps->IMEI, &kv_speed_ex);
					flag_se = 1;
					// printf("0 end \n");
				}
			}

			// }
		}

		if (flag != all_speed) {
			cJSON_Delete(obj);
			return -3;
		}
	}
#endif	/* if 1 */
	// (cJSON_GetArrayItem(son_speed,0))->valueint;

	p_gps->max_speed = (cJSON_GetArrayItem(son_speed, 0))->valueint;
	p_gps->min_speed = (cJSON_GetArrayItem(son_speed, 0))->valueint;
	p_gps->avg_speed = 0;

	for (i = 0; i < all_speed; i++) {
		arr_speed = cJSON_GetArrayItem(son_speed, i);
		int speed = arr_speed->valueint;

		if (speed > p_gps->max_speed) {
			p_gps->max_speed = speed;
		}

		if (speed < p_gps->min_speed) {
			p_gps->min_speed = speed;
		}

		p_gps->avg_speed = p_gps->avg_speed + speed;
	}

	p_gps->avg_speed = p_gps->avg_speed / all_speed;

	/*parse GPSTime*/
	son = cJSON_GetObjectItem(obj, "GPSTime");

	if (NULL == son) {
		x_printf(E, "data has no GPSTime!");
		goto fail;
	}

	all = cJSON_GetArraySize(son);

	if ((all <= 0) || (all > 15)) {
		x_printf(E, "Failed to decode GPS, the array number of GPSTime is not right!\n");
		goto fail;
	}

	p_gps->start_time = (cJSON_GetArrayItem(son, all - 1))->valueint;
	p_gps->end_time = (cJSON_GetArrayItem(son, 0))->valueint;
	/*parse  longitude*/
	son = cJSON_GetObjectItem(obj, "longitude");

	if (NULL == son) {
		x_printf(E, "data has no longitude!");
		goto fail;
	}

	all = cJSON_GetArraySize(son);

	if ((all <= 0) || (all > 15)) {
		x_printf(E, "Failed to decode GPS, the array number of longitude is not right!\n");
		goto fail;
	}

	p_gps->longitude = (cJSON_GetArrayItem(son, 0))->valuedouble;
	/*parse  latitude*/
	son = cJSON_GetObjectItem(obj, "latitude");

	if (NULL == son) {
		x_printf(E, "data has no latitude!");
		goto fail;
	}

	all = cJSON_GetArraySize(son);

	if ((all <= 0) || (all > 15)) {
		x_printf(E, "Failed to decode GPS, the array number of latitude is not right!\n");
		goto fail;
	}

	p_gps->latitude = (cJSON_GetArrayItem(son, 0))->valuedouble;
	/*parse  direction*/
	son = cJSON_GetObjectItem(obj, "direction");

	if (NULL == son) {
		x_printf(E, "data has no direction!");
		goto fail;
	}

	all = cJSON_GetArraySize(son);

	if ((all <= 0) || (all > 15)) {
		x_printf(E, "Failed to decode GPS, the array number of  direction is not right!\n");
		goto fail;
	}

	p_gps->direction = (cJSON_GetArrayItem(son, 0))->valueint;
	/*parse  altitude*/
	son = cJSON_GetObjectItem(obj, "altitude");

	if (NULL == son) {
		x_printf(E, "data has no altitude!");
		goto fail;
	}

	all = cJSON_GetArraySize(son);

	if ((all <= 0) || (all > 15)) {
		x_printf(E, "Failed to decode GPS, the array number of  altitude is not right!\n");
		goto fail;
	}

	p_gps->altitude = (cJSON_GetArrayItem(son, 0))->valueint;

	cJSON_Delete(obj);
	return 0;

fail:
	cJSON_Delete(obj);
	return -1;
}

