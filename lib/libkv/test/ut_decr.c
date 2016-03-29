#include "ut_public.h"



void case_cmd_decr()
{
        /**
         * decr no_exist key
         * decr string-type key
         * decr character string-type key
         * decr list-type key
         */
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);
        
        ans = kv_ask(handler, "decr no_exist_key", strlen("decr no_exist_key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "-1");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "get no_exist_key", "-1");

        /* string-type key */
        _case_answer_string_equal(handler, "set key 123", "OK");

        ans = kv_ask(handler, "decr key", strlen("decr key"));
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "122");
        kv_answer_release(ans);

        /** character string-type key */
        _case_answer_string_equal(handler, "set key1 abcd", "OK");

        ans = kv_ask(handler, "decr key1", strlen("decr key1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        kv_answer_release(ans);

        ans = kv_ask(handler, "get key1", strlen("get key1"));
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "abcd");
        kv_answer_release(ans);
        
        /* list-type key */
        _case_answer_string_equal(handler, "lpush lkey a b c", "3");

        ans = kv_ask(handler, "decr lkey", strlen("decr lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "set key1 abc", "OK");
        ans = kv_ask(handler, "decr lkey", strlen("decr lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_decr_multi_handler()
{
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        _case_answer_string_equal(handlerA, "set key10 10", "OK");
        _case_answer_string_equal(handlerB, "set key100 100", "OK");
        _case_answer_string_equal(handlerC, "set key1000 1000", "OK");

        ans = kv_ask(handlerA, "decr key10", strlen("decr key10"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "9");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "decr key100", strlen("decr key100"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "99");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "decr key1000", strlen("decr key1000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "999");
        kv_answer_release(ans);

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_decr);
        UTIL_ADD_CASE_SHORT(case_cmd_decr_multi_handler);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
