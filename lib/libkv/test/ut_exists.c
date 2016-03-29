#include "ut_public.h"



void case_cmd_exists_normal()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "set key value", "OK");
        
        ans = kv_ask(handler, "exists key", strlen("exists key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handler, "exists key_noexist", strlen("exists key_noexist"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "del key", "1");

        ans = kv_ask(handler, "exists key", strlen("exists key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_exists_invalid_cmd()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "exist key", strlen("exist key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_exists_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "exists", strlen("exists"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);

        ans = kv_ask(handler, "exists invalid1 invalid2", strlen("exists invalid1 invalid2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_exists_multi_handler()
{
        int i;
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;
        char buf[10000+128];
        
        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        for (i = 1; i <= 30000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d value", i);
                _case_answer_string_equal(handlerA, buf, "OK");
                _case_answer_string_equal(handlerB, buf, "OK");
                _case_answer_string_equal(handlerC, buf, "OK");
        }

        for (i = 1; i <= 30000; i++) {
                snprintf(buf, sizeof(buf), "exists key%d", i);
                ans = kv_ask(handlerA, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerB, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerC, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
        }

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);  
}

void case_cmd_exists_stress()
{
        int i;
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;
        char buf[10000+128];
        
        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        for (i = 1; i <= 30000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d value%d", i);
                _case_answer_string_equal(handlerA, buf, "OK");
                _case_answer_string_equal(handlerB, buf, "OK");
                _case_answer_string_equal(handlerC, buf, "OK");

                snprintf(buf, sizeof(buf), "lpush lkey%d a b c", i);
                _case_answer_string_equal(handlerA, buf, "3");
                _case_answer_string_equal(handlerB, buf, "3");
                _case_answer_string_equal(handlerC, buf, "3");
        }

        for (i = 1; i <= 30000; i++) {
                snprintf(buf, sizeof(buf), "exists key%d", i);
                ans = kv_ask(handlerA, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerB, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerC, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                snprintf(buf, sizeof(buf), "exists lkey%d", i);
                ans = kv_ask(handlerA, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerB, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerC, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
                
        }

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);  
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_exists_normal);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_exists_stress);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
