#include "kv_road.h"
#include "kv_cache.h"
#include "rr_def.h"
#include "string.h"
#include <stdlib.h>

extern kv_cache *g_kv_cache;

void parse_road_section(char *section, SECKV_ROAD *road)
{
	int     k = 0, cur = 0;
	char    d[3] = ":;";

	x_printf(D, ">> %s\n", section);
	char *t = strtok(section, d);

	while (t != NULL) {
		if (k == 6) {
			cur += 1;
			k = 0;
		}

		switch (k)
		{
			case 0:
				road->road_sec[cur].begin = atoi(t);
				break;

			case 1:
				road->road_sec[cur].end = atoi(t);
				break;

			case 2:
				road->road_sec[cur].avg_speed = atoi(t);
				break;

			case 3:
				road->road_sec[cur].endtime = atol(t);
				break;

			case 4:
				road->road_sec[cur].longitude = strtod(t, NULL);
				break;

			case 5:
				road->road_sec[cur].latitude = strtod(t, NULL);
				break;

			default:
				break;
		}
		k++;
		t = strtok(NULL, d);
	}

	road->road_sec[99].longitude = road->road_sec[cur].longitude;
	road->road_sec[99].latitude = road->road_sec[cur].latitude;
	road->road_sec[99].avg_speed = road->road_sec[cur].avg_speed;
	road->road_sec[99].endtime = road->road_sec[cur].endtime;

	road->road_sec[98].longitude = road->road_sec[cur - 1].longitude;
	road->road_sec[98].latitude = road->road_sec[cur - 1].latitude;

	x_printf(D, ">>99 %f, %f, %f, %f", road->road_sec[cur].longitude, road->road_sec[cur].latitude, road->road_sec[99].longitude, road->road_sec[99].latitude);
	x_printf(D, ">>98 %f, %f, %f, %f", road->road_sec[cur - 1].longitude, road->road_sec[cur - 1].latitude, road->road_sec[98].longitude, road->road_sec[98].latitude);
}

void parse_road_section_s(char *section, SECKV_ROAD *road)
{
	int     k = 0, cur = 0;
	char    d[3] = ":;";

	x_printf(D, ">> %s\n", section);
	char    *token = section;
	char    *t = NULL;

	while ((t = strsep(&token, d)) != NULL) {
		if (k == 6) {
			cur += 1;
			k = 0;
		}

		switch (k)
		{
			case 0:
				road->road_sec[cur].begin = atoi(t);
				break;

			case 1:
				road->road_sec[cur].end = atoi(t);
				break;

			case 2:
				road->road_sec[cur].avg_speed = atoi(t);
				break;

			case 3:
				road->road_sec[cur].endtime = atol(t);
				break;

			case 4:
				road->road_sec[cur].longitude = strtod(t, NULL);
				break;

			case 5:
				road->road_sec[cur].latitude = strtod(t, NULL);
				break;

			default:
				break;
		}
		k++;
	}

	road->road_sec[99].longitude = road->road_sec[cur - 1].longitude;
	road->road_sec[99].latitude = road->road_sec[cur - 1].latitude;
	road->road_sec[99].avg_speed = road->road_sec[cur - 1].avg_speed;
	road->road_sec[99].endtime = road->road_sec[cur - 1].endtime;

	road->road_sec[98].longitude = road->road_sec[cur - 2].longitude;
	road->road_sec[98].latitude = road->road_sec[cur - 2].latitude;

	x_printf(D, ">>99 %f, %f, %f, %f", road->road_sec[cur - 1].longitude, road->road_sec[cur - 1].latitude, road->road_sec[99].longitude, road->road_sec[99].latitude);
	x_printf(D, ">>98 %f, %f, %f, %f", road->road_sec[cur - 2].longitude, road->road_sec[cur - 2].latitude, road->road_sec[98].longitude, road->road_sec[98].latitude);
}

void assembly_road_section(char *ret, SECKV_ROAD *road)
{
	int     i;
	char    sub[100] = "";

	for (i = 0; i < road->sec_num; i++) {
		memset(sub, 0, sizeof(sub));
		sprintf(sub, "%d:%d:%d:%ld:%lf:%lf;", road->road_sec[i].begin, road->road_sec[i].end, road->road_sec[i].avg_speed, road->road_sec[i].endtime, road->road_sec[i].longitude, road->road_sec[i].latitude);
		// sprintf(sub, "%d:%d:%d;", road->road_sec[i].begin, road->road_sec[i].end, road->road_sec[i].avg_speed);
		strcat(ret, sub);
	}

	memset(sub, 0, sizeof(sub));
	sprintf(sub, "%d:%d:%d:%ld:%lf:%lf;", road->road_sec[98].begin, road->road_sec[98].end, road->road_sec[98].avg_speed, road->road_sec[98].endtime, road->road_sec[98].longitude, road->road_sec[98].latitude);
	strcat(ret, sub);
	memset(sub, 0, sizeof(sub));
	sprintf(sub, "%d:%d:%d:%ld:%lf:%lf;", road->road_sec[99].begin, road->road_sec[99].end, road->road_sec[99].avg_speed, road->road_sec[99].endtime, road->road_sec[99].longitude, road->road_sec[99].latitude);
	strcat(ret, sub);
}

void show_road_section(char *ret, SECKV_ROAD *road)
{
	int     i;
	char    sub[100] = "";

	sprintf(sub, "%lf:%lf:%d:%ld;", road->road_sec[98].longitude, road->road_sec[98].latitude, road->road_sec[98].avg_speed, road->road_sec[98].endtime);
	strcat(ret, sub);

	for (i = 0; i < road->sec_num; i++) {
		memset(sub, 0, sizeof(sub));
		sprintf(sub, "%lf:%lf:%d:%ld;", road->road_sec[i].longitude, road->road_sec[i].latitude, road->road_sec[i].avg_speed, road->road_sec[i].endtime);
		strcat(ret, sub);
	}

	memset(sub, 0, sizeof(sub));
	sprintf(sub, "%lf:%lf:%d:%ld;", road->road_sec[99].longitude, road->road_sec[99].latitude, road->road_sec[99].avg_speed, road->road_sec[99].endtime);
	strcat(ret, sub);
}

void single_road_section(char *ret, SECKV_ROAD *road)
{
	int     i;
	char    sub[100] = "";

	for (i = 0; i < 2; i++) {
		memset(sub, 0, sizeof(sub));
		sprintf(sub, "%lf:%lf:%d:%ld;", road->road_sec[i].longitude, road->road_sec[i].latitude, road->road_sec[i].avg_speed, road->road_sec[i].endtime);
		strcat(ret, sub);
	}
}

int get_roadINFO_from_kv(long roadid, SECKV_ROAD *kv_road)
{
	char            buff[1024] = { 0 };
	kv_answer_t     *ans;
	int             flag = SUC_ROAD;

	memset(buff, '\0', sizeof(buff));
	sprintf(buff, "hmget %ld section IMEI max_speed avg_speed end_time used_time old_roadID citycode countycode sec_num", roadid);
	char kv_hash[128];
	sprintf(kv_hash, "%ld", roadid);
	ans = kv_cache_ask(g_kv_cache, kv_hash, buff);

	if (ans->errnum != ERR_NONE) {
		if (ans->errnum == ERR_NIL) {
			x_printf(D, "this IMEI has not data!\n");
			flag = NIL_ROAD;
			goto end;
		}

		x_printf(E, "Failed to get redis_IMEI, errnum is %d, err is %s\n", ans->errnum, ans->err);
		flag = ERR_ROAD;
		goto end;
	}

	unsigned long           len = kv_answer_length(ans);
	kv_answer_iter_t        *iter = NULL;

	if (len != 1) {
		int                     i = 0;
		kv_answer_value_t       *value = NULL;
		iter = kv_answer_get_iter(ans, ANSWER_HEAD);
		kv_answer_rewind_iter(ans, iter);

		while ((value = kv_answer_next(iter)) != NULL) {
			i = i + 1;
			switch (i)
			{
				case 1:
					parse_road_section_s(value->ptr, kv_road); break;

				case 2:
					kv_road->IMEI = atoll(value->ptr); break;

				case 3:
					kv_road->max_speed = atoi(value->ptr); break;

				case 4:
					kv_road->avg_speed = atoi(value->ptr); break;

				case 5:
					kv_road->end_time = atoll(value->ptr); break;

				case 6:
					kv_road->used_time = atoll(value->ptr); break;

				case 7:
					kv_road->old_roadID = atoll(value->ptr); break;

				case 8:
					kv_road->citycode = atoll(value->ptr); break;

				case 9:
					kv_road->countycode = atoll(value->ptr); break;

				case 10:
					kv_road->sec_num = atoi(value->ptr); break;

				default:
					break;
			}
		}
	}

	kv_answer_release_iter(iter);
end:
	kv_answer_release(ans);
	return flag;
}

int set_secroad_to_kv(SECKV_ROAD *kv_road)
{
	char            buff[10240] = { 0 };
	char            ret[2048] = { 0 };
	kv_answer_t     *ans = NULL;

	assembly_road_section(ret, kv_road);
	memset(buff, '\0', sizeof(buff));
	sprintf(buff,
		"hmset %ld section %s IMEI %lld max_speed %d avg_speed %d end_time %ld used_time %ld old_roadID %ld citycode %d countycode %d sec_num %d",
		kv_road->old_roadID, ret, kv_road->IMEI, kv_road->max_speed,
		kv_road->avg_speed, kv_road->end_time,
		kv_road->used_time, kv_road->old_roadID, kv_road->citycode,
		kv_road->countycode, kv_road->sec_num);

	char kv_hash[128];
	sprintf(kv_hash, "%ld", kv_road->old_roadID);
	ans = kv_cache_ask(g_kv_cache, kv_hash, buff);

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "redis command error errnum is %d\t err is %s\n", ans->errnum, ans->err);
		kv_answer_release(ans);
		return -1;
	}

	kv_answer_release(ans);
	return 0;
}

int set_roadID_to_kv(SECKV_ROAD *kv_roadID)
{
	char            buff[10240] = { 0 };
	kv_answer_t     *ans = NULL;

	memset(buff, '\0', sizeof(buff));
	sprintf(buff,
		"hmset %ld IMEI %lld max_speed %d avg_speed %d end_time %ld used_time %ld old_roadID %ld citycode %d countycode %d",
		kv_roadID->old_roadID, kv_roadID->IMEI, kv_roadID->max_speed,
		kv_roadID->avg_speed, kv_roadID->end_time,
		kv_roadID->used_time, kv_roadID->old_roadID, kv_roadID->citycode,
		kv_roadID->countycode);

	char kv_hash[128];
	sprintf(kv_hash, "%ld", kv_roadID->old_roadID);
	ans = kv_cache_ask(g_kv_cache, kv_hash, buff);

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "redis command error errnum is %d\t err is %s\n", ans->errnum, ans->err);
		kv_answer_release(ans);
		return -1;
	}

	kv_answer_release(ans);
	return 0;
}

