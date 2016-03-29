#include "ut_public.h"



void case_cmd_set_normal()
{
        kv_handler_t *handler;
        char cmd[1024];
        kv_answer_t *ans;

        handler = kv_create(NULL);
        
        _case_answer_string_equal(handler, "set key value", "OK");
        _case_answer_string_equal(handler, "set key value", "OK");
        _case_answer_string_equal(handler, "set key value_new", "OK");
        _case_answer_string_equal(handler, "set key_new value", "OK");

        ans = kv_ask(handler, "set key_new value", strlen("set key_new value"));
        CU_ASSERT_STRING_EQUAL("OK", kv_answer_value_to_string(kv_answer_first_value(ans)));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_set_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);
        
        ans = kv_ask(handler, "set", strlen("set"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(ans->head, NULL);
        CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
        kv_answer_release(ans);

        ans = kv_ask(handler, "set key", strlen("set key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(ans->head, NULL);
        CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
        kv_answer_release(ans);

        ans = kv_ask(handler, "set key value", strlen("set key value"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        ans = kv_ask(handler, "set key value1 more", strlen("set key value1 more"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(ans->head, NULL);
        CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_set_invalid_type()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);
        
        _case_answer_string_equal(handler, "lpush lkey a b c", "3");
        _case_answer_string_equal(handler, "set lkey l_to_s", "OK");

        ans = kv_ask(handler, "lpush lkey d e", strlen("lpush lkey d e"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_set_multi_handler()
{
        int i;
        char buf[128];
        kv_handler_t *handlerA, *handlerB, *handlerC;
        kv_answer_t *ans;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);
        
        for (i = 1; i <= 30000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d %d", i, i);
                ans = kv_ask(handlerA, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
                kv_answer_release(ans);
        }

        for (i = 1; i <= 20000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d %d", i, i);
                ans = kv_ask(handlerB, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
                kv_answer_release(ans);
        }

        for (i = 1; i <= 10000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d %d", i, i);
                ans = kv_ask(handlerC, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
                kv_answer_release(ans);
        }

        _case_answer_string_equal(handlerA, "dbsize", "30000");
        _case_answer_string_equal(handlerB, "dbsize", "20000");
        _case_answer_string_equal(handlerC, "dbsize", "10000");

        for (i = 1; i <= 10000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "lpush lkey%d %d", i, i);
                ans = kv_ask(handlerA, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
        }

        _case_answer_string_equal(handlerA, "dbsize", "40000");

        _case_answer_string_equal(handlerA, "flushdb", "OK");
        _case_answer_string_equal(handlerA, "flushdb", "OK");
        _case_answer_string_equal(handlerA, "flushdb", "OK");

        _case_answer_string_equal(handlerA, "dbsize", "0");
        _case_answer_string_equal(handlerA, "dbsize", "0");
        _case_answer_string_equal(handlerA, "dbsize", "0");
        
        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}

void case_cmd_set_stress()
{
        /* key1 ~ key10000, value -> random */
        /* set */
        kv_handler_t *handler;
        kv_answer_t *ans;
        kv_answer_iter_t *iter;
        kv_answer_value_t *value;
        int i;
        static char cmd[10000+128];
        case_data_t *cd[10000];

        CU_ASSERT_PTR_NOT_EQUAL((handler = kv_create(NULL)), NULL);
        for (i = 1; i <= 10000 ; i++) {
                int bytes = snprintf(cmd, sizeof(cmd), "set key%d ", i);
                cd[i-1] = generator_rand_str(i);
                memcpy(cmd + bytes, cd[i-1]->ptr, cd[i-1]->ptrlen);
                
                ans = kv_ask(handler, cmd, bytes + cd[i-1]->ptrlen);
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
                kv_answer_release(ans);
        }

        for (i = 1; i <= 10000; i++) {
                case_data_release(cd[i-1]);
        }
        _case_answer_string_equal(handler, "dbsize", "10000");
        
        kv_destroy(handler);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_set_normal);
        UTIL_ADD_CASE_SHORT(case_cmd_set_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_set_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_set_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_set_stress);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
