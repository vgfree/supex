#include "user_info.h"
#include "kv_cache.h"
#include "entry.h"

int user_info_save(user_info_t *p_user)
{
        char buff[1240] = { 0 };

        memset(buff, '\0', sizeof(buff));
        kv_answer_t     *ans = NULL;
        sprintf(buff,  "set %s:user_info %ld", p_user->IMEI, (unsigned long)p_user);
        ans = kv_cache_ask(g_kv_cache, p_user->IMEI, buff);

        if (ans->errnum != ERR_NONE) {
                x_printf(E, "Failed to set redis IMEI, errnum is %d, err is %s !\n", ans->errnum, ans->err);
                kv_answer_release(ans);
                return -1;
        }

        return 0;
}

int user_info_add(user_info_t *p_user, road_info_t *p_road)
{
        int i;
        for(i = MLOCATE_ROAD_DEEP - 1; i > 0; i--){
                p_user->roads[i] = p_user->roads[i - 1];
        }
        p_user->roads[0] = *p_road;

        if(p_user->count < MLOCATE_ROAD_DEEP)
                p_user->count++;

        return 0;
}

user_info_t* user_info_get(char *p_imei)
{
        char            buff[1024] = { 0 };
        kv_answer_t     *ans;

        memset(buff, '\0', sizeof(buff));
        sprintf(buff, "get %s:user_info", p_imei);

        ans = kv_cache_ask(g_kv_cache, p_imei, buff);

        if (ans->errnum != ERR_NONE) {
                if (ans->errnum == ERR_NIL) {
                        x_printf(D, "this IMEI has not data!\n");
                }
                x_printf(E, "Failed to get redis_IMEI, errnum is %d, err is %s\n", ans->errnum, ans->err);
        }

        unsigned long           len = kv_answer_length(ans);
        kv_answer_iter_t        *iter = NULL;
        if(len == 0) {
                user_info_t *p_user = (user_info_t *)malloc(sizeof(user_info_t));
                memset(p_user, 0 , sizeof(user_info_t));
                strncpy(p_user->IMEI, p_imei, IMEI_LEN);
                user_info_save(p_user);
                kv_answer_release(ans);

                return p_user;
        }

        kv_answer_value_t       *value = NULL;
        iter = kv_answer_get_iter(ans, ANSWER_HEAD);
        kv_answer_rewind_iter(ans, iter);
        value = kv_answer_next(iter);
        unsigned long pt = strtol((char *)(value->ptr), NULL, 10);
        kv_answer_release(ans);

        user_info_t *p_user = (user_info_t *)pt;

        return p_user;
}
