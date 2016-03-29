#include "ut_public.h"



void case_cmd_incr()
{
        /**
         * incr no_exist key
         * incr string-type key
         * incr list-type key
         */
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);
        
        ans = kv_ask(handler, "incr no_exist_key", strlen("incr no_exist_key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "get no_exist_key", "1");

        /* string-type key */
        _case_answer_string_equal(handler, "set key 123", "OK");

        ans = kv_ask(handler, "incr key", strlen("incr key"));
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "124");
        kv_answer_release(ans);

        /* list-type key */
        _case_answer_string_equal(handler, "lpush lkey a b c", "3");

        ans = kv_ask(handler, "incr lkey", strlen("incr lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "set key1 abc", "OK");
        ans = kv_ask(handler, "incr lkey", strlen("incr lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void case_cmd_incr_multi_handler()
{
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        _case_answer_string_equal(handlerA, "set key10 10", "OK");
        _case_answer_string_equal(handlerB, "set key100 100", "OK");
        _case_answer_string_equal(handlerC, "set key1000 1000", "OK");

        ans = kv_ask(handlerA, "incr key10", strlen("incr key10"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "11");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "incr key100", strlen("incr key100"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "101");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "incr key1000", strlen("incr key1000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1001");
        kv_answer_release(ans);

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}


#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_incr);
        UTIL_ADD_CASE_SHORT(case_cmd_incr_multi_handler);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
