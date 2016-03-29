#include "ut_public.h"



void case_cmd_sismember()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "sismember skey memb", strlen("sismember skey memb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "sadd skey s1 s2 s3", "3");

        ans = kv_ask(handler, "sismember skey s1", strlen("sismember skey s1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "sismember skey s2", strlen("sismember skey s2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handler, "sismember skey s3", strlen("sismember skey s3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handler, "sismember skey s4", strlen("sismember skey s4"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        /** digital */
        _case_answer_string_equal(handler, "sadd skey_dig 11 22 33", "3");
        
        ans = kv_ask(handler, "sismember skey_dig 11", strlen("sismember skey_dig 11"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "sismember skey_dig 22", strlen("sismember skey_dig 22"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handler, "sismember skey_dig 33", strlen("sismember skey_dig 33"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handler, "sismember skey_dig 44", strlen("sismember skey_dig 44"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_sismember_invalid_cmd()
{
        kv_handler_t *handler;
        kv_answer_t *ans;


        handler = kv_create(NULL);
        CU_ASSERT(handler != NULL);

        ans = kv_ask(handler, "sismeber key mem", strlen("sismeber key mem"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        kv_answer_release(ans);

        ans = kv_ask(handler, "ismember key mem", strlen("ismember key mem"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_sismember_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);
        CU_ASSERT(handler != NULL);

        ans = kv_ask(handler, "sismember", strlen("sismember"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        ans = kv_ask(handler, "sismember key", strlen("sismember key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        ans = kv_ask(handler, "sismember key v1 v2", strlen("sismember key v1 v2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_sismember_invalid_type()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);
        CU_ASSERT(handler != NULL);

        _case_answer_string_equal(handler, "set key value", "OK");

        ans = kv_ask(handler, "sismember key value", strlen("sismember key value"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_sismember_multi_handler()
{
        kv_handler_t *handlerA, *handlerB, *handlerC;
        kv_answer_t *ans;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        _case_answer_string_equal(handlerA, "sadd skey s1 s2", "2");
        _case_answer_string_equal(handlerB, "sadd skey s3 s4", "2");
        _case_answer_string_equal(handlerC, "sadd skey s5 s6", "2");

        ans = kv_ask(handlerA, "sismember skey s1", strlen("sismember skey s1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerA, "sismember skey s2", strlen("sismember skey s2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "sismember skey s3", strlen("sismember skey s3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "sismember skey s4", strlen("sismember skey s4"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "sismember skey s5", strlen("sismember skey s5"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "sismember skey s6", strlen("sismember skey s6"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerA, "sismember skey s3", strlen("sismember skey s3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "sismember skey s1", strlen("sismember skey s1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "sismember skey s4", strlen("sismember skey s4"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "sadd skey_dig 1 2", "2");
        _case_answer_string_equal(handlerB, "sadd skey_dig 3 4", "2");
        _case_answer_string_equal(handlerC, "sadd skey_dig 5 6", "2");

        ans = kv_ask(handlerA, "sismember skey_dig 1", strlen("sismember skey_dig 1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "sismember skey_dig 3", strlen("sismember skey_dig 3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "sismember skey_dig 6", strlen("sismember skey_dig 6"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        
        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_sismember_db()
{
        int i;
        kv_handler_t *handlerA, *handlerB;
        kv_answer_t *ans;
        char buf[1024];

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);

        _case_answer_string_equal(handlerA, "select 1", "OK");
        _case_answer_string_equal(handlerA, "sadd xkey s1", "1");
        _case_answer_string_equal(handlerA, "select 2", "OK");
        _case_answer_string_equal(handlerA, "sadd xkey s2", "1");

        snprintf(buf, sizeof(buf), "sismember xkey s1", i, i);
        ans = kv_ask(handlerA, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "select 1", "OK");
        snprintf(buf, sizeof(buf), "sismember xkey s1", i, i);
        ans = kv_ask(handlerA, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);
        

        for (i = 0; i < 16; i++) {
                snprintf(buf, sizeof(buf), "select %d", i);
                _case_answer_string_equal(handlerA, buf, "OK");
                _case_answer_string_equal(handlerB, buf, "OK");
                
                snprintf(buf, sizeof(buf), "sadd %d %d", i, i);
                _case_answer_string_equal(handlerA, buf, "1");
                _case_answer_string_equal(handlerB, buf, "1");

                snprintf(buf, sizeof(buf), "sismember %d %d", i, i);
                ans = kv_ask(handlerA, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerB, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
        }
        
        kv_destroy(handlerA);
        kv_destroy(handlerB);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_sismember);
        UTIL_ADD_CASE_SHORT(case_cmd_sismember_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_sismember_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_sismember_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_sismember_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_sismember_db);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
