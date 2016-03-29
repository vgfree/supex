#include "ut_public.h"





void case_cmd_get()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        ans = kv_ask(handler, "set key value", strlen("set key value"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "value");
        kv_answer_release(ans);

        ans = kv_ask(handler, "get key key_more", strlen("get key key_more"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);

        ans = kv_ask(handler, "lpush lkey a b c", strlen("lpush lkey a b c"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "3");
        kv_answer_release(ans);

        ans = kv_ask(handler, "get lkey", strlen("get lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_get_invalid_type()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "set key value", "OK");
        _case_answer_string_equal(handler, "lpush lkey a b c", "3");
        _case_answer_string_equal(handler, "sadd skey d e f", "3");

        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "value");
        kv_answer_release(ans);

        ans = kv_ask(handler, "get lkey", strlen("get lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);

        ans = kv_ask(handler, "get skey", strlen("get skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_get_nil()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "get noexist", strlen("get noexist"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void case_cmd_get_stress()
{
        int i;
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;
        char buf[10000+128];
        char bufB[10000+128];
        
        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        
        for (i = 1; i <= 30000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d %d", i, i);
                _case_answer_string_equal(handlerA, buf, "OK");

                snprintf(buf, sizeof(buf), "get key%d", i);
                snprintf(bufB, sizeof(buf), "%d", i);
                _case_answer_string_equal(handlerA, buf, bufB);
        }

        for (i = 1; i <= 20000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d %d", i, i);
                _case_answer_string_equal(handlerB, buf, "OK");

                snprintf(buf, sizeof(buf), "get key%d", i);
                snprintf(bufB, sizeof(buf), "%d", i);
                _case_answer_string_equal(handlerB, buf, bufB);
        }

        for (i = 1; i <= 10000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d %d", i, i);
                _case_answer_string_equal(handlerC, buf, "OK");

                snprintf(buf, sizeof(buf), "get key%d", i);
                snprintf(bufB, sizeof(buf), "%d", i);
                _case_answer_string_equal(handlerC, buf, bufB);
        }

        _case_answer_string_equal(handlerA, "dbsize", "30000");
        _case_answer_string_equal(handlerB, "dbsize", "20000");
        _case_answer_string_equal(handlerC, "dbsize", "10000");

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);  
}


#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_get);
        UTIL_ADD_CASE_SHORT(case_cmd_get_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_get_nil);
        UTIL_ADD_CASE_SHORT(case_cmd_get_stress);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
