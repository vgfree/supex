#include "kv_imei.h"
#include "kv_cache.h"
#include <stdlib.h>
#include "rr_def.h"

extern kv_cache *g_kv_cache;

int get_IMEI_from_kv(char *IMEI, KV_IMEI *kv_IMEI)
{
	char            buff[10240] = { 0 };
	kv_answer_t     *ans;
	int             flag = SUC_IMEI;

	memset(buff, '\0', sizeof(buff));
	sprintf(buff, "hmget %s count max_speed_num max_speed str_time end_time roadID IMEI citycode countycode direction rt start_lon start_lat end_lon end_lat", IMEI);
	char kv_hash[128];
	sprintf(kv_hash, "%s", IMEI);
	ans = kv_cache_ask(g_kv_cache, kv_hash, buff);

	if (ans->errnum != ERR_NONE) {
		if (ans->errnum == ERR_NIL) {
			x_printf(D, "this IMEI has not data!\n");
			flag = NIL_IMEI;
			goto end;
		}

		x_printf(E, "Failed to get redis_IMEI, errnum is %d, err is %s\n", ans->errnum, ans->err);
		flag = ERR_IMEI;
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
					kv_IMEI->count = atoi(value->ptr); break;

				case 2:
					kv_IMEI->max_speed_num = atoi(value->ptr); break;

				case 3:
					kv_IMEI->max_speed = atoi(value->ptr); break;

				case 4:
					kv_IMEI->str_time = atoll(value->ptr); break;

				case 5:
					kv_IMEI->end_time = atoll(value->ptr); break;

				case 6:
					kv_IMEI->roadID = atoll(value->ptr); break;

				case 7:
					strcpy(kv_IMEI->IMEI,value->ptr); break;

				case 8:
					kv_IMEI->citycode = atoll(value->ptr); break;

				case 9:
					kv_IMEI->countycode = atoll(value->ptr); break;

				case 10:
					kv_IMEI->direction = atoi(value->ptr); break;

				case 11:
					kv_IMEI->rt = atoi(value->ptr); break;

				case 12:
					kv_IMEI->start_lon = strtod(value->ptr, NULL); break;

				case 13:
					kv_IMEI->start_lat = strtod(value->ptr, NULL); break;

				case 14:
					kv_IMEI->end_lon = strtod(value->ptr, NULL); break;

				case 15:
					kv_IMEI->end_lat = strtod(value->ptr, NULL); break;

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

int set_IMEI_to_kv(KV_IMEI *kv_IMEI)
{
	char buff[1240] = { 0 };

	memset(buff, '\0', sizeof(buff));
	int             flag = 0;
	kv_answer_t     *ans = NULL;
	sprintf(buff,
		"hmset %s count %d max_speed_num %d max_speed %d str_time %ld end_time %ld roadID %ld IMEI %s citycode %ld countycode %ld direction %d rt %d start_lon %f start_lat %f end_lon %f end_lat %f",
		kv_IMEI->IMEI, kv_IMEI->count, kv_IMEI->max_speed_num, kv_IMEI->max_speed,
		kv_IMEI->str_time, kv_IMEI->end_time, kv_IMEI->roadID, kv_IMEI->IMEI,
		kv_IMEI->citycode, kv_IMEI->countycode, kv_IMEI->direction, kv_IMEI->rt, kv_IMEI->start_lon, kv_IMEI->start_lat, kv_IMEI->end_lon, kv_IMEI->end_lat);
	char kv_hash[128];
        sprintf(kv_hash, "%s", kv_IMEI->IMEI);
        ans = kv_cache_ask(g_kv_cache, kv_hash, buff);

        if (ans->errnum != ERR_NONE) {
                x_printf(E, "Failed to hmset redis IMEI, errnum is %d, err is %s !\n", ans->errnum, ans->err);
                flag = ERR_IMEI;
                goto end;
        }

end:
        kv_answer_release(ans);
        return flag;
}

int delete_IMEI_from_kv(KV_IMEI *kv_IMEI)
{
        char buff[1240] = { 0 };

        memset(buff, '\0', sizeof(buff));
        int             flag = 0;
        kv_answer_t     *ans = NULL;
        sprintf(buff, "hdel %s count max_speed_num max_speed str_time end_time roadID IMEI citycode countycode direction rt", kv_IMEI->IMEI);
        char kv_hash[128];
        sprintf(kv_hash, "%s", kv_IMEI->IMEI);
        ans = kv_cache_ask(g_kv_cache, kv_hash, buff);

        if (ans->errnum != ERR_NONE) {
                x_printf(E, "Failed to hdel redis IMEI, errnum is %d, err is %s !\n", ans->errnum, ans->err);
                flag = ERR_IMEI;
                goto end;
        }

end:    
        kv_answer_release(ans);
        return flag;
}

/*
int set_IMEI_expire(KV_IMEI *kv_IMEI, int time)
{
        char buff[1240] = { 0 };

        memset(buff, '\0', sizeof(buff));
        int             flag = 0;
        kv_answer_t     *ans = NULL;
        sprintf(buff, "expire %ld ", kv_IMEI->IMEI, time);
        char kv_hash[128];
        sprintf(kv_hash, "%ld", kv_IMEI->IMEI);
        ans = kv_cache_ask(g_kv_cache, kv_hash, buff);

        if (ans->errnum != ERR_NONE) {
                x_printf(E, "Failed to expire redis IMEI, errnum is %d, err is %s !\n", ans->errnum, ans->err);
                flag = ERR_IMEI;
                goto end;
        }

end:    
        kv_answer_release(ans);
        return flag;
}
*/
