#include "ut_public.h"



void case_cmd_incrby()
{
        /**
         * incr valid number to no_exist key
         * incr valid number to exist key(string-type)
         * incr valid number to exist key(lpush-type)
         * incr invalid number to no_exist key
         * incr invalid number to exist key(string-type)
         * incr invalid number to exist key(lpush-type)
         */
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);
       
        /** no exist key */
        ans = kv_ask(handler, "incrby no_exist_key 123", strlen("incrby no_exist_key 123"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "123");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "get no_exist_key", "123");
        
        /** exist key */
        _case_answer_string_equal(handler, "set key 123", "OK");

        ans = kv_ask(handler, "incrby key 123", strlen("incrby key 123"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "246");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "get key", "246");

        /** list */
        _case_answer_string_equal(handler, "lpush lkey a b c", "3");

        ans = kv_ask(handler, "incrby lkey 456", strlen("incr lkey 456"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "flushdb", "OK");

        /**
         * incrby invalid number 
         */
        /** no exist key */
        ans = kv_ask(handler, "incrby no_exist_key abcd", strlen("incrby no_exist_key abcd"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        kv_answer_release(ans);

        /** exist key */
        _case_answer_string_equal(handler, "set key 123", "OK");

        ans = kv_ask(handler, "incrby key abcd", strlen("incr key abcd"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "get key", "123");

        /** invalid value */
        _case_answer_string_equal(handler, "set key1 abcd", "OK");
        
        ans = kv_ask(handler, "incrby key1 123", strlen("incrby key1 123"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        kv_answer_release(ans);
        
        /** list */
        _case_answer_string_equal(handler, "lpush lkey a b c", "3");

        ans = kv_ask(handler, "incrby lkey 456", strlen("incr lkey 456"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void case_cmd_incrby_multi_handler()
{
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        _case_answer_string_equal(handlerA, "set key10 10", "OK");
        _case_answer_string_equal(handlerB, "set key100 100", "OK");
        _case_answer_string_equal(handlerC, "set key1000 1000", "OK");

        ans = kv_ask(handlerA, "incrby key10 5", strlen("incrby key10 5"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "15");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "incrby key100 66", strlen("incrby key100 66"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "166");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "incrby key1000 777", strlen("incrby key1000 777"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1777");
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "dbsize", "1");
        _case_answer_string_equal(handlerA, "dbsize", "1");
        _case_answer_string_equal(handlerA, "dbsize", "1");
        
        _case_answer_string_equal(handlerA, "get key10", "15");
        _case_answer_string_equal(handlerB, "get key100", "166");
        _case_answer_string_equal(handlerC, "get key1000", "1777");

        
        _case_answer_string_equal(handlerA, "set key1 1", "OK");
        _case_answer_string_equal(handlerB, "set key2 2", "OK");
        _case_answer_string_equal(handlerC, "set key3 3", "OK");

        ans = kv_ask(handlerA, "incrby key1 1000", strlen("incrby key1 1000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1001");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "incrby key2 1000", strlen("incrby key2 1000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1002");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "incrby key3 1000", strlen("incrby key3 1000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1003");
        kv_answer_release(ans);

        
        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_incrby);
        UTIL_ADD_CASE_SHORT(case_cmd_incrby_multi_handler);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
