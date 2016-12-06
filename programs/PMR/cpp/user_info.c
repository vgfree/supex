#include "user_info.h"
#include "kv_cache.h"
#include "entry.h"

int user_info_save(user_info_t *p_user)
{
	char buff[1240] = { 0 };

	memset(buff, '\0', sizeof(buff));
	sprintf(buff, "set %s:user_info %ld", p_user->IMEI, (unsigned long)p_user);
	kv_handler_t *handler = kv_cache_spl(g_kv_cache, p_user->IMEI, buff);
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
                kv_handler_release(handler);
		return -1;
        }

	kv_handler_release(handler);
	return 0;
}

int user_info_add(user_info_t *p_user, road_info_t *p_road)
{
	int i;

	for (i = MLOCATE_ROAD_DEEP - 1; i > 0; i--) {
		p_user->roads[i] = p_user->roads[i - 1];
	}

	p_user->roads[0] = *p_road;

	if (p_user->count < MLOCATE_ROAD_DEEP) {
		p_user->count++;
	}

	return 0;
}

user_info_t *user_info_get(char *p_imei)
{
	char            buff[1024] = { 0 };

	memset(buff, '\0', sizeof(buff));
	sprintf(buff, "get %s:user_info", p_imei);

	kv_handler_t *handler = kv_cache_spl(g_kv_cache, p_imei, buff);
	kv_answer_t *ans = &handler->answer;
	
	if (ERR_NONE != ans->errnum) {
                x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
        }

	unsigned long           len = answer_length(ans);
	if (len == 1) {
		kv_answer_value_t *value = answer_head_value(ans);
		if (answer_value_look_type(value) == VALUE_TYPE_NIL) {
			x_printf(D, "this IMEI has not data!\n");
			user_info_t *p_user = (user_info_t *)malloc(sizeof(user_info_t));
			memset(p_user, 0, sizeof(user_info_t));
			strncpy(p_user->IMEI, p_imei, IMEI_LEN);
			user_info_save(p_user);

			kv_handler_release(handler);
			return p_user;
		} else {
			char ptr[128] = {0};
			memcpy(ptr, answer_value_look_addr(value), answer_value_look_size(value));
			unsigned long pt = strtol((char *)ptr, NULL, 10);
			kv_handler_release(handler);

			user_info_t *p_user = (user_info_t *)pt;

			return p_user;
		}
	}
	return NULL;
}

