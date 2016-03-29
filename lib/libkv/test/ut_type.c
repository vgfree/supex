#include "ut_public.h"




void case_cmd_type()
{
        kv_handler_t *handler;
        kv_answer_t* ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "type key", strlen("type key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "none");
        kv_answer_release(ans);
                
        /** string */
        _case_answer_string_equal(handler, "set key value", "OK");

        ans = kv_ask(handler, "type key", strlen("type key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "string");
        kv_answer_release(ans);

        /** list */
        _case_answer_string_equal(handler, "lpush lkey a bb ccc", "3");
        ans = kv_ask(handler, "type lkey", strlen("type lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "list");
        kv_answer_release(ans);

        /** set */
        _case_answer_string_equal(handler, "sadd skey s1 s2", "2");
        ans = kv_ask(handler, "type skey", strlen("type skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "set");
        kv_answer_release(ans);

#if 0
        /** zset */
        _case_answer_string_equal(handler, "zadd zkey 1 z1 2 z2 3 z3", "3");
        ans = kv_ask(handler, "type zkey", strlen("type zkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "zset");
        kv_answer_release(ans);
#endif
        /** hash */
        _case_answer_string_equal(handler, "hset hkey f1 v1", "1");
        ans = kv_ask(handler, "type hkey", strlen("type hkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "hash");
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_type_invalid_cmd()
{
        kv_handler_t *handler;
        kv_answer_t* ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "typ key", strlen("typ key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_type_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t* ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "type", strlen("type"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);

        ans = kv_ask(handler, "type key value", strlen("typ key value"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_type_multi_handler()
{
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB;
        

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);

        ans = kv_ask(handlerA, "type key", strlen("type key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "none");
        kv_answer_release(ans);
        
        ans = kv_ask(handlerB, "type key", strlen("type key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "none");
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "set key value", "OK");
        _case_answer_string_equal(handlerA, "type key", "string");
        _case_answer_string_equal(handlerB, "type key", "none");

        _case_answer_string_equal(handlerA, "dbsize", "1");
        _case_answer_string_equal(handlerB, "dbsize", "0");

        _case_answer_string_equal(handlerB, "lpush lkey a b c", "3");
        ans = kv_ask(handlerB, "type lkey", strlen("type lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "list");
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "dbsize", "1");
        _case_answer_string_equal(handlerB, "dbsize", "1");

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}





#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();

        UTIL_ADD_CASE_SHORT(case_cmd_type);
        UTIL_ADD_CASE_SHORT(case_cmd_type_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_type_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_type_multi_handler);
                            
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
