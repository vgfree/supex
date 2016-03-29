#include "ut_public.h"



void case_cmd_decrby()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        /** no exist key */
        ans = kv_ask(handler, "decrby no_exist_key 123", strlen("decrby no_exist_key 123"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "-123");
        kv_answer_release(ans);

        ans = kv_ask(handler, "get no_exist_key", strlen("get no_exist_key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "-123");
        kv_answer_release(ans);
        
        /** exist key */
        _case_answer_string_equal(handler, "set key 123", "OK");

        ans = kv_ask(handler, "decrby key 12345678901234567890123456789012345678901234567890", strlen("decrby key 12345678901234567890123456789012345678901234567890"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "decrby key abcd", strlen("decrby key abcd"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "decrby key 124", strlen("decrby key 124"));
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "-1");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "get key", "-1");

        /** invalid number */
        ans = kv_ask(handler, "decrby no_exist_key1 abcd", strlen("decrby no_exist_key1 abcd"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        kv_answer_release(ans);

        /** invalid value */
        _case_answer_string_equal(handler, "set key2 abcd", "OK");
        
        ans = kv_ask(handler, "decrby key2 123", strlen("decrby key2 123"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        kv_answer_release(ans);

        /** list */
        _case_answer_string_equal(handler, "lpush lkey a b c", "3");

        ans = kv_ask(handler, "decrby lkey 456", strlen("decrby lkey 456"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_decrby_multi_handler()
{
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        _case_answer_string_equal(handlerA, "set key10 10", "OK");
        _case_answer_string_equal(handlerB, "set key100 100", "OK");
        _case_answer_string_equal(handlerC, "set key1000 1000", "OK");

        ans = kv_ask(handlerA, "decrby key10 5", strlen("decrby key10 5"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "5");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "decrby key100 66", strlen("decrby key100 66"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "34");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "decrby key1000 777", strlen("decrby key1000 777"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "223");
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "dbsize", "1");
        _case_answer_string_equal(handlerA, "dbsize", "1");
        _case_answer_string_equal(handlerA, "dbsize", "1");
        
        _case_answer_string_equal(handlerA, "get key10", "5");
        _case_answer_string_equal(handlerB, "get key100", "34");
        _case_answer_string_equal(handlerC, "get key1000", "223");

        
        _case_answer_string_equal(handlerA, "set key1 1", "OK");
        _case_answer_string_equal(handlerB, "set key2 2", "OK");
        _case_answer_string_equal(handlerC, "set key3 3", "OK");

        ans = kv_ask(handlerA, "decrby key1 1000", strlen("decrby key1 1000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "-999");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "decrby key2 1000", strlen("decrby key2 1000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "-998");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "decrby key3 1000", strlen("decrby key3 1000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "-997");
        kv_answer_release(ans);
        
        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}




#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_decrby);
        UTIL_ADD_CASE_SHORT(case_cmd_decrby_multi_handler);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
