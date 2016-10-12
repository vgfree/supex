#include "utils.h"
#include "kv_cache.h"
#include "kv_speed.h"
#include <string.h>

extern kv_cache *g_kv_cache;

int get_speed_from_kv(char *IMEI, kv_speed *kv_speed_ex)
{
	char            buff[10240] = { 0 };
	kv_answer_t     *ans;
	int             flag = SUC_IMEI;

	memset(buff, '\0', sizeof(buff));
	sprintf(buff, "hmget %s tokenCode  sign", IMEI);

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
					strncpy(kv_speed_ex->tokenCode, value->ptr, strlen(value->ptr)); break;

				case 2:
					kv_speed_ex->sign = atoi(value->ptr); break;

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

int set_speed_to_kv(char *IMEI, kv_speed *kv_speed_ex)
{
	char buff[1024] = { 0 };

	memset(buff, '\0', sizeof(buff));

	int             flag = 0;
	kv_answer_t     *ans = NULL;

	sprintf(buff, "hmset %s tokenCode %s sign %d", IMEI, kv_speed_ex->tokenCode, kv_speed_ex->sign);

	char kv_hash[128] = { '\0' };
	sprintf(kv_hash, "%s", IMEI);
	ans = kv_cache_ask(g_kv_cache, kv_hash, buff);

	if (ans->errnum != ERR_NONE) {
		x_printf(E, "Failed to hmset redis imei&token, errnum is %d, err is %s !\n", ans->errnum, ans->err);
		flag = ERR_IMEI;
		goto end;
	}

end:
	kv_answer_release(ans);
	return flag;
}

