#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <json.h>
#include <cJSON.h>

#include "jtt_body.h"
#include "log.h"

#include "jtt_json_to_gps.h"

#define SAMPLE_STEP 5		/*采样步长,每SAMPLE_STEP个点采集一个点*/

/*待确定问题：参数的单位，取值范围,数据类型*/
typedef struct
{
	char            imei[16];
	unsigned        longitude;	/*经度*/
	unsigned        latitude;	/*纬度*/
	unsigned        speed;		/*速度*/
	int             altitude;	/*海拔*/
	int             direction;	/*方向*/
	unsigned        GPSTime;	/*时间*/
} GPS;

static int time_compare(const void *big, const void *small)
{
	GPS     *b = (GPS *)big;
	GPS     *s = (GPS *)small;

	if (b->GPSTime == s->GPSTime) {
		return 0;
	}

	return (b->GPSTime > s->GPSTime) ? 1 : -1;
}

static int gps_to_jtt(GPS gps, BODY_GPS *body_gps)
{
	time_t          t;
	struct tm       *date;
	unsigned short  mask = 0x00ff;

	strncpy(body_gps->imei, gps.imei, sizeof(body_gps->imei) - 1);
	body_gps->lon = gps.longitude;
	body_gps->lat = gps.latitude;
	body_gps->vec1 = gps.speed;

	/*停车时，方向角为-1;按特殊约定转换为0*/
	if (-1 == gps.direction) {
		gps.direction = 0;
		body_gps->state = (1 << 4) | body_gps->state;
	}

	body_gps->direction = gps.direction;
	/*809没有定义负海拔，特殊约定：将负海拔转为无符号型进行传输，接收方收到后转换为有符号型数据*/
	body_gps->altitude = (unsigned short)gps.altitude;

	t = gps.GPSTime;
	date = localtime(&t);

	if (NULL == date) {
		return -1;
	}

	body_gps->date[0] = date->tm_mday;
	body_gps->date[1] = date->tm_mon + 1;
	body_gps->date[2] = ((unsigned short)date->tm_year + 1900) >> 8;
	body_gps->date[3] = ((unsigned short)date->tm_year + 1900) & mask;
	body_gps->time[0] = date->tm_hour;
	body_gps->time[1] = date->tm_min;
	body_gps->time[2] = date->tm_sec;

	// printf("[%d][%d][%d][%d][%d][%d]\n",date->tm_year+1900,body_gps->date[1], body_gps->date[0], body_gps->time[0],body_gps->time[1],body_gps->time[2]);

	return 0;
}

static int json_to_gps(cJSON *str, char *imei, GPS *gps, int *n)
{
	cJSON   *json = NULL;
	cJSON   *value = NULL;
	cJSON   *node = NULL;
	int     i = 0;
	int     j = 0;

	json = str;

	if (!imei) {	/*没有imei为实时数据*/
		value = cJSON_GetObjectItem(json, "IMEI");

		if (!value) {
			log_info(LOG_E, "没有 IMEI\n");
			return -1;
		}

		*n = 1;	/*一个包只采集一个点*/
		strncpy(gps->imei, value->valuestring, sizeof(gps->imei) - 1);
	}

	value = cJSON_GetObjectItem(json, "longitude");

	if (!value || (value->type != cJSON_Array)) {
		log_info(LOG_E, "没有 longitude or not array\n");
		return -1;
	}

	i = 0;
	node = value->child;

	while (node && i < *n) {/*cJSON_GetArrayItem 每次从头结点开始，速度较慢,不用*/
		(gps + i)->longitude = (unsigned)(node->valuedouble * 1000000u);

		for (j = 0; j < SAMPLE_STEP && node; j++) {
			node = node->next;
		}

		i++;
	}

	value = cJSON_GetObjectItem(json, "latitude");

	if (!value || (value->type != cJSON_Array)) {
		log_info(LOG_E, "没有 latitude or not array\n");
		return -1;
	}

	i = 0;
	node = value->child;

	while (node && i < *n) {	/*cJSON_GetArrayItem 每次从头结点开始，速度较慢,不用*/
		(gps + i)->latitude = (unsigned)(node->valuedouble * 1000000u);

		for (j = 0; j < SAMPLE_STEP && node; j++) {
			node = node->next;
		}

		i++;
	}

	value = cJSON_GetObjectItem(json, "speed");

	if (!value || (value->type != cJSON_Array)) {
		log_info(LOG_E, "没有 speed or not array\n");
		return -1;
	}

	i = 0;
	node = value->child;

	while (node && i < *n) {
		(gps + i)->speed = (unsigned)node->valueint;

		for (j = 0; j < SAMPLE_STEP && node; j++) {
			node = node->next;
		}

		i++;
	}

	value = cJSON_GetObjectItem(json, "direction");

	if (!value || (value->type != cJSON_Array)) {
		log_info(LOG_E, "没有 direction or not array\n");
		return -1;
	}

	i = 0;
	node = value->child;

	while (node && i < *n) {
		(gps + i)->direction = node->valueint;

		for (j = 0; j < SAMPLE_STEP && node; j++) {
			node = node->next;
		}

		i++;
	}

	value = cJSON_GetObjectItem(json, "altitude");

	if (!value || (value->type != cJSON_Array)) {
		log_info(LOG_E, "没有 altitude or not array\n");
		return -1;
	}

	i = 0;
	node = value->child;

	while (node && i < *n) {
		(gps + i)->altitude = node->valueint;

		for (j = 0; j < SAMPLE_STEP && node; j++) {
			node = node->next;
		}

		i++;
	}

	value = cJSON_GetObjectItem(json, "GPSTime");

	if (!value || (value->type != cJSON_Array)) {
		log_info(LOG_E, "没有 GPSTime or not array\n");
		return -1;
	}

	i = 0;
	node = value->child;

	while (node && i < *n) {
		(gps + i)->GPSTime = (unsigned)node->valueint;

		for (j = 0; j < SAMPLE_STEP && node; j++) {
			node = node->next;
		}

		i++;
	}

	return 0;
}

/*
 * 由于不知补传数据的GPS点数,所以，采用malloc分配空间，外部free
 * free条件：返回0 && *gps==NULL
 *
 */
static int json_extra_to_gps(cJSON *str, char *imei, GPS **gps, int *n)
{
	cJSON   *json = NULL;
	cJSON   *obj = NULL;
	cJSON   *value = NULL;
	int     i = 0;

	json = str;
	obj = cJSON_GetObjectItem(json, "extragps");

	if (!obj) {
		return 0;
	}

	if (obj->type != cJSON_Object) {
		log_info(LOG_E, "extragps not object\n");
		return -1;
	}

	value = cJSON_GetObjectItem(obj, "longitude");

	if (!value || (value->type != cJSON_Array)) {
		log_info(LOG_E, "没有 longitude or not array\n");
		return -1;
	}

	*n = cJSON_GetArraySize(value);
	*n = *n / SAMPLE_STEP;		/*SAMPLE_STEP个点选一个点*/
	*gps = (GPS *)calloc(*n, sizeof(GPS));

	if (!*gps) {
		log_info(LOG_E, "calloc error\n");
		*n = 0;
		return -1;
	}

	for (i = 0; i < *n; i++) {
		strncpy((*gps + i)->imei, imei, sizeof((*gps + i)->imei) - 1);
	}

	if (-1 == json_to_gps(obj, imei, *gps, n)) {
		free(*gps);
		*gps = NULL;
		log_info(LOG_E, "json_to_gps error\n");
		return -1;
	}

	return 0;
}

/*
 *功能：将json字符串转换为gps数据
 *	 gps:实时数据
 *	*extra:补传数据
 *
 * 说明：实时数据空间由外部确定，补传数据的空间由内部malloc,外部free
 * 补传数据判断条件：*extra != NULL
 * */
int json_to_gps_data(cJSON *str, BODY_GPS *bgps, int *n_gps, BODY_GPS **bextra, int *n_extra)
{
	int     i;
	GPS     *gps = NULL;
	GPS     *ex = NULL;
	GPS     **extra = &ex;

	*bextra = NULL;
	gps = (GPS *)calloc(*n_gps, sizeof(GPS));

	if (NULL == gps) {
		log_info(LOG_E, "内存分配错\n");
		return -1;
	}

	if (-1 == json_to_gps(str, NULL, gps, n_gps)) {
		free(gps);
		return -1;
	}

	//	qsort(gps,*n_gps,sizeof(GPS),time_compare);
	for (i = 0; i < *n_gps; i++) {
		if (-1 == gps_to_jtt(*(gps + i), (bgps + i))) {
			free(gps);
			return -1;
		}
	}

	if (-1 == json_extra_to_gps(str, gps[0].imei, extra, n_extra)) {
		free(gps);
		return -1;
	}

	free(gps);

	if (NULL == *extra) {
		return 0;
	}

	*bextra = (BODY_GPS *)calloc(*n_extra, sizeof(BODY_GPS));

	if (NULL == *bextra) {
		free(*extra);
		return -1;
	}

	qsort(*extra, *n_extra, sizeof(GPS), time_compare);

	for (i = 0; i < *n_extra; i++) {
		if (-1 == gps_to_jtt(*(*extra + i), (*bextra + i))) {
			free(*extra);
			free(*bextra);
			return -1;
		}
	}

	free(*extra);
	return 0;
}

