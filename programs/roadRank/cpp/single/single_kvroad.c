#include "kv_road.h"
#include "kv_cache.h"
#include <stdlib.h>

extern kv_cache *g_kv_cache;

// 数据处理在函数外
int set_roadID_to_kv(KV_ROADID *kv_roadID)
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

