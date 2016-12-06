#include "utils.h"
#include "kv_cache.h"
#include "kv_speed.h"
#include <string.h>

extern kv_cache *g_kv_cache;

int get_speed_from_kv(char *IMEI, kv_speed *kv_speed_ex)
{
	char            buff[10240] = { 0 };
	int             flag = SUC_IMEI;

	memset(buff, '\0', sizeof(buff));
	sprintf(buff, "hmget %s tokenCode  sign", IMEI);

	char kv_hash[128];
	sprintf(kv_hash, "%s", IMEI);
	kv_handler_t *handler = kv_cache_spl(g_kv_cache, kv_hash, buff);
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		flag = ERR_IMEI;
		goto end;
        }

	unsigned long           len = answer_length(ans);
	if (len != 1) {
		int                     i = 0;
		kv_answer_iter_t        *iter = answer_iter_make(ans, ANSWER_HEAD);

		kv_answer_value_t       *value;
		while ((value = answer_iter_next(iter)) != NULL) {
			if (answer_value_look_type(value) == VALUE_TYPE_NIL) {
				x_printf(D, "this IMEI has not data!\n");
				answer_iter_free(iter);
				flag = NIL_IMEI;
				goto end;
			}

			i = i + 1;
			char ptr[128] = {0};
			memcpy(ptr, answer_value_look_addr(value), answer_value_look_size(value));
			switch (i)
			{
				case 1:
					strncpy(kv_speed_ex->tokenCode, answer_value_look_addr(value), answer_value_look_size(value)); break;

				case 2:
					kv_speed_ex->sign = atoi(ptr); break;

				default:
					break;
			}

		}


		answer_iter_free(iter);
	}

end:
	kv_handler_release(handler);
	return flag;
}


int set_speed_to_kv(char *IMEI, kv_speed *kv_speed_ex)
{
	char buff[1024] = { 0 };

	memset(buff, '\0', sizeof(buff));

	int             flag = 0;

	sprintf(buff, "hmset %s tokenCode %s sign %d", IMEI, kv_speed_ex->tokenCode, kv_speed_ex->sign);

	char kv_hash[128] = { '\0' };
	sprintf(kv_hash, "%s", IMEI);

	kv_handler_t *handler = kv_cache_spl(g_kv_cache, kv_hash, buff);
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		flag = ERR_IMEI;
		goto end;
        }

end:
	kv_handler_release(handler);
	return flag;
}

