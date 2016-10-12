#include "kv_road_v2.h"
#include "kv_cache.h"
#include "rr_def.h"
#include "string.h"
#include <stdlib.h>

extern kv_cache *g_kv_cache;

void show_road_section(char *ret, SECKV_ROAD *road)
{
	int     i;
	char    sub[100] = "";
	int     ret_size = BUFF_SEC_LEN;
	int     threshold = 60;

	sprintf(sub, "%lf:%lf:%d:%ld:%d:%d:%s;", road->road_sec[98].longitude, road->road_sec[98].latitude, road->road_sec[98].avg_speed, road->road_sec[98].endtime, road->road_sec[98].max_speed, road->road_sec[98].used_time, road->road_sec[98].imei);
	strcat(ret, sub);

	for (i = 0; i < road->sec_num; i++) {
		memset(sub, 0, sizeof(sub));
		sprintf(sub, "%lf:%lf:%d:%ld:%d:%d:%s;", road->road_sec[i].longitude, road->road_sec[i].latitude, road->road_sec[i].avg_speed, road->road_sec[i].endtime, road->road_sec[i].max_speed, road->road_sec[i].used_time, road->road_sec[i].imei);

		if (ret_size - strlen(ret) > threshold) {
			strcat(ret, sub);
		} else {
			x_printf(E, "stack size: %ld %ld %d\n", strlen(ret), strlen(sub), ret_size);
		}
	}

	memset(sub, 0, sizeof(sub));
	sprintf(sub, "%lf:%lf:%d:%ld:%d:%d:%s;", road->road_sec[99].longitude, road->road_sec[99].latitude, road->road_sec[99].avg_speed, road->road_sec[99].endtime, road->road_sec[99].max_speed, road->road_sec[99].used_time, road->road_sec[99].imei);
	x_printf(D, "stack size: %ld %ld %d\n", strlen(ret), strlen(sub), ret_size);

	if (ret_size - strlen(ret) > threshold) {
		strcat(ret, sub);
	} else {
		x_printf(E, "stack size: %ld %ld %d\n", strlen(ret), strlen(sub), ret_size);
	}
}

void single_road_section(char *ret, SECKV_ROAD *road)
{
	int     i;
	char    sub[100] = "";

	for (i = 0; i < 2; i++) {
		memset(sub, 0, sizeof(sub));
		sprintf(sub, "%lf:%lf:%d:%ld:%d:%d:%s;", road->road_sec[i].longitude, road->road_sec[i].latitude, road->road_sec[i].avg_speed, road->road_sec[i].endtime, road->road_sec[i].max_speed, road->road_sec[i].used_time, road->road_sec[i].imei);
		strcat(ret, sub);
	}
}

int roadsec_info_get(long roadid, SECKV_ROAD **roadsec)
{
	char            buff[1024] = { 0 };
	kv_answer_t     *ans;
	int             flag = SUC_ROAD;

	memset(buff, '\0', sizeof(buff));
	sprintf(buff, "get %ld:road_info", roadid);
	char kv_hash[128];
	sprintf(kv_hash, "%ld", roadid);
	ans = kv_cache_ask(g_kv_cache, kv_hash, buff);

	if (ans->errnum != ERR_NONE) {
		if (ans->errnum == ERR_NIL) {
			x_printf(D, "this road has not section info! %lu\n", sizeof(SECKV_ROAD));
			*roadsec = (SECKV_ROAD *)malloc(sizeof(SECKV_ROAD));

			if ((*roadsec) == NULL) {
				x_printf(E, "malloc error\n");
				flag = ERR_ROAD;
				goto end;
			}

			memset(*roadsec, 0, sizeof(SECKV_ROAD));
			(*roadsec)->old_roadID = roadid;
			// AO_SpinLockInit(&(*roadsec)->locker, false);
			AO_LockInit(&(*roadsec)->locker, false);

			// AO_SpinLock(&roadsec->locker);
			if (roadsec_info_save(*roadsec) == -1) {
				x_printf(E, "roadsec_info_save error\n");
				free(*roadsec);
				flag = ERR_ROAD;
				goto end;
			}

			flag = NIL_ROAD;
			goto end;
		}

		x_printf(E, "Failed to get redis_roadsec, errnum is %d, err is %s\n", ans->errnum, ans->err);
		flag = ERR_ROAD;
		goto end;
	}

	unsigned long           len = kv_answer_length(ans);
	kv_answer_iter_t        *iter = NULL;

	/*
	 *   if(len == 0) {
	 *        x_printf(E, "len ====== 0\n");
	 * roadsec = (SECKV_ROAD *)malloc(sizeof(SECKV_ROAD));
	 *        if((*roadsec) == NULL) {
	 *                x_printf(E, "malloc error\n");
	 *                flag = ERR_ROAD;
	 *                goto end;
	 *        }
	 *        memset(*roadsec, 0 , sizeof(SECKV_ROAD));
	 *        (*roadsec)->old_roadID = roadid;
	 *        AO_SpinLockInit(&(*roadsec)->locker, false);
	 *        //AO_SpinLock(&roadsec->locker);
	 *        if( roadsec_info_save(*roadsec) == -1 ) {
	 *                x_printf(E, "roadsec_info_save error\n");
	 *                free(*roadsec);
	 *                flag = ERR_ROAD;
	 *                goto end;
	 *        }
	 *
	 *        flag = NIL_ROAD;
	 *        goto end;
	 *   }
	 */
	if (len == 1) {
		kv_answer_value_t *value = NULL;
		iter = kv_answer_get_iter(ans, ANSWER_HEAD);
		kv_answer_rewind_iter(ans, iter);
		value = kv_answer_next(iter);
		unsigned long pt = strtol((char *)(value->ptr), NULL, 10);
		*roadsec = (SECKV_ROAD *)pt;
	}

	kv_answer_release_iter(iter);
end:
	kv_answer_release(ans);
	return flag;
}

int roadsec_info_save(SECKV_ROAD *kv_road)
{
	char    buff[1240] = "";
	char    kv_hash[128] = "";

	// 存之前 先取一次 保证不分配两次内存
	kv_answer_t *ans2 = NULL;

	sprintf(buff, "get %ld:road_info", kv_road->old_roadID);
	sprintf(kv_hash, "%ld", kv_road->old_roadID);
	ans2 = kv_cache_ask(g_kv_cache, kv_hash, buff);

	if (ans2->errnum != ERR_NONE) {
		if (ans2->errnum == ERR_NIL) {
			memset(buff, '\0', sizeof(buff));
			memset(kv_hash, '\0', sizeof(kv_hash));
			kv_answer_t *ans = NULL;
			sprintf(buff, "set %ld:road_info %ld", kv_road->old_roadID, (unsigned long)kv_road);
			sprintf(kv_hash, "%ld", kv_road->old_roadID);
			ans = kv_cache_ask(g_kv_cache, kv_hash, buff);

			if (ans->errnum != ERR_NONE) {
				x_printf(E, "Failed to set redis roadsec, errnum is %d, err is %s !\n", ans->errnum, ans->err);
				kv_answer_release(ans);
				kv_answer_release(ans2);
				return -1;
			}

			x_printf(D, "save road_info succ\n");
			kv_answer_release(ans);
			kv_answer_release(ans2);
			return 0;
		}

		x_printf(E, "Failed to set redis roadsec, errnum is %d, err is %s !\n", ans2->errnum, ans2->err);
		kv_answer_release(ans2);
		return -1;
	}

	x_printf(E, "atomic save road:%ld\n", kv_road->old_roadID);
	kv_answer_release(ans2);
	return -1;
}

int set_roadID_to_kv(SECKV_ROAD *kv_roadID)
{
	char            buff[1024] = { 0 };
	kv_answer_t     *ans = NULL;

	memset(buff, '\0', sizeof(buff));
	sprintf(buff,
		"hmset %ld IMEI %s max_speed %d avg_speed %d end_time %ld used_time %ld old_roadID %ld citycode %d countycode %d",
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

