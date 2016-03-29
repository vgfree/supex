#include <unistd.h>
#include "ut_public.h"



void case_cmd_pexpire_get()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "set key value", "OK");

        ans = kv_ask(handler, "pexpire key 1000", strlen("pexpire key 1000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        usleep((useconds_t)0.5E6);
        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "value");
        kv_answer_release(ans);

        usleep((useconds_t)0.6E6);

        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "dbsize", "0");

        _case_answer_string_equal(handler, "set key value", "OK");
        
        ans = kv_ask(handler, "pexpire key 2000", strlen("pexpire key 2000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        usleep((useconds_t)2001E3);

        _case_answer_string_equal(handler, "del key", "0");

        _case_answer_string_equal(handler, "set key value", "OK");
        
        ans = kv_ask(handler, "del key", strlen("del key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_pexpire_list()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);
        
        _case_answer_string_equal(handler, "lpush lkey a b c", "3");

        ans = kv_ask(handler, "pexpire lkey 1000", strlen("pexpire lkey 1000"));
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        _case_answer_length_equal(handler, "lrange lkey 0 -1", 3);

        usleep((useconds_t)1.1E6);
        
        ans = kv_ask(handler, "lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void case_cmd_pexpire_set()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "sadd skey a b c", "3");

        ans = kv_ask(handler, "pexpire skey 1000", strlen("pexpire skey 1000"));
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        _case_answer_length_equal(handler, "smembers skey", 3);

        usleep((useconds_t)1.1E6);
        ans = kv_ask(handler, "smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void case_cmd_pexpire_without_key()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "pexpire key 10000", strlen("pexpire key 10000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_pexpire_invalid_cmd()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);
        
        ans = kv_ask(handler, "expir key 10000", strlen("expir key 10000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_pexpire_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);
        
        ans = kv_ask(handler, "pexpire", strlen("pexpire"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans); 
        
        ans = kv_ask(handler, "pexpire key", strlen("pexpire key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);

        ans = kv_ask(handler, "pexpire key 10000 20000", strlen("pexpire key 10000 20000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);

        ans = kv_ask(handler, "pexpire key 123456789012345678901234567890", strlen("pexpire key 123456789012345678901234567890"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_pexpire_multi_handler()
{
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        _case_answer_string_equal(handlerA, "set keyA valueA", "OK");
        _case_answer_string_equal(handlerB, "set keyB valueB", "OK");
        _case_answer_string_equal(handlerC, "set keyC valueC", "OK");

        _case_answer_string_equal(handlerA, "get keyA", "valueA");
        _case_answer_string_equal(handlerB, "get keyB", "valueB");
        _case_answer_string_equal(handlerC, "get keyC", "valueC");
        
        ans = kv_ask(handlerA, "pexpire keyA 1000", strlen("pexpire keyA 1000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "pexpire keyB 3000", strlen("pexpire keyB 3000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "pexpire keyC 5000", strlen("pexpire keyC 5000"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "get keyA", "valueA");
        _case_answer_string_equal(handlerB, "get keyB", "valueB");
        _case_answer_string_equal(handlerC, "get keyC", "valueC");

        usleep((useconds_t)1.1E6);
        ans = kv_ask(handlerA, "get keyA", strlen("get keyA"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        _case_answer_string_equal(handlerB, "get keyB", "valueB");
        _case_answer_string_equal(handlerC, "get keyC", "valueC");

        sleep(2);
        ans = kv_ask(handlerB, "get keyB", strlen("get keyB"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);
        
        _case_answer_string_equal(handlerC, "get keyC", "valueC");

        sleep(2);
        ans = kv_ask(handlerC, "get keyC", strlen("get keyC"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}

void case_cmd_pexpire_stress()
{
        int i;
        kv_answer_t *ans;
        kv_handler_t *handler;
        static char buf[10000+32];

        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "set key value", "OK");
        
        for (i = 10000000; i >= 1000; i--) {
                snprintf(buf, sizeof(buf), "pexpire key %d", i);
                ans = kv_ask(handler, buf, strlen(buf));
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
        }

        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "value");
        kv_answer_release(ans);

        usleep((useconds_t)1.1E6);
        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_get);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_list);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_set);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_pexpire_stress);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
