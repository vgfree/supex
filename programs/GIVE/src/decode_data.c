/*
 * Author       : chenzutao
 * Date         : 2015-10-15
 * Function     : decode_data.c
 */

#include "decode_data.h"

int decode_data(gps_data_t **value, const char *data)
{
	if (!data) {
		x_printf(E, "gps data is null !\n");
		return -1;
	}

	cJSON *obj = cJSON_Parse(data);

	if (!obj) {
		x_printf(E, "[** FAILED **] Failed to parse data: %s, error:%s\n", data, cJSON_GetErrorPtr());
		return -1;
	}

	*value = (gps_data_t *)calloc(sizeof(gps_data_t), 1);
	cJSON *temp = NULL;
	temp = cJSON_GetObjectItem(obj, "tokenCode");

	if (!temp) {
		x_printf(E, "Failed to decode tokenCode .\n");
		free(*value), *value = NULL;
		cJSON_Delete(obj);
		return -1;
	}

	(*value)->id = (char *)calloc(sizeof(char), strlen(temp->valuestring) + 1);
	strcpy((*value)->id, temp->valuestring);

	temp = cJSON_GetObjectItem(obj, "latitude");

	if (!temp) {
		x_printf(E, "Failed to decode latitude .\n");
		free_data(*value);
		cJSON_Delete(obj);
		return -1;
	}

	int len = cJSON_GetArraySize(temp);

	if ((len < 0) || (len > 5)) {
		x_printf(E, "array length out of range, length:%d.\n", len);
		free_data(*value);
		cJSON_Delete(obj);
		return 0;
	}

	int     i;
	cJSON   *arr = NULL;

	for (i = 0; i < len; i++) {
		arr = cJSON_GetArrayItem(temp, i);

		if (arr) {
			(*value)->lat[i] = (unsigned int)(arr->valuedouble * 1000000);
		}
	}

	temp = cJSON_GetObjectItem(obj, "longitude");

	if (!temp) {
		x_printf(E, "Failed to decode longitude .\n");
		free_data(*value);
		cJSON_Delete(obj);
		return -1;
	}

	for (i = 0; i < len; i++) {
		arr = cJSON_GetArrayItem(temp, i);

		if (arr) {
			(*value)->lng[i] = (unsigned int)(arr->valuedouble * 1000000);
		}
	}

	temp = cJSON_GetObjectItem(obj, "speed");

	if (!temp) {
		x_printf(E, "Failed to decode speed .\n");
		free_data(*value);
		cJSON_Delete(obj);
		return -1;
	}

	for (i = 0; i < len; i++) {
		arr = cJSON_GetArrayItem(temp, i);

		if (arr) {
			(*value)->speed[i] = arr->valueint;
		}
	}

	temp = cJSON_GetObjectItem(obj, "direction");

	if (!temp) {
		x_printf(E, "Failed to decode direction .\n");
		free_data(*value);
		cJSON_Delete(obj);
		return -1;
	}

	for (i = 0; i < len; i++) {
		arr = cJSON_GetArrayItem(temp, i);

		if (arr) {
			(*value)->angle[i] = arr->valueint;
		}
	}

	temp = cJSON_GetObjectItem(obj, "GPSTime");

	if (!temp) {
		x_printf(E, "Failed to decode GPSTime .\n");
		free_data(*value);
		cJSON_Delete(obj);
		return -1;
	}

	for (i = 0; i < len; i++) {
		arr = cJSON_GetArrayItem(temp, i);

		if (arr) {
			(*value)->gpsTime[i] = arr->valueint;
		}
	}

	cJSON_Delete(obj);
	return len;
}

int free_data(gps_data_t *value)
{
	if (!value) {
		return -1;
	}

	if (value->id) {
		free(value->id), value->id = NULL;
	}

	free(value), value = NULL;
	return 0;
}

#if 0
int main()
{
	char data[] = "{\"longitude\":[121.3613973,121.361327,121.3612662,121.3612118,121.3611633],\"latitude\":[31.2253355,31.2253577,31.2253755,31.2253873,31.225387],\"IMEI\":\"299563123936856\",\"model\":\"SG900\",\"collect\":true,\"speed\":[26,22,19,16,14],\"tokenCode\":\"yuzwlqU8lX\",\"accountID\":\"kxl1QuHKCD\",\"GPSTime\":[1407210985,1407210985,1407210985,1407210985,1407210985],\"direction\":[111,111,108,93,82],\"altitude\":[5,5,5,5,5]}";

	gps_data_t      *value = NULL;
	int             ret = decode_data(&value, data);

	if (ret < 0) {
		x_printf(E, "Failed to decode data .\n");
	}

	int i;

	for (i = 0; i < ret; i++) {
		x_printf(I, "id:%s, lat:%d, lng:%d, speed:%d, angle:%d, gpsTime:%d\n", value->id, value->lat[i], value->lng[i], value->speed[i], value->angle[i], value->gpsTime[i]);
	}

	free_data(value);
	return 0;
}
#endif	/* if 0 */

