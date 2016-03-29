#include <unistd.h>
#include "ut_public.h"




void case_cmd_expireat_get()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        char cmd[128];
        
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "set key value", "OK");

        int write = snprintf(cmd, sizeof(cmd), "expireat key %ld", time(NULL) + 2);
        ans = kv_ask(handler, cmd, strlen(cmd));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        sleep(1);
        _case_answer_string_equal(handler, "get key", "value");

        usleep((useconds_t)1.1E6);

        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "dbsize", "0");

        _case_answer_string_equal(handler, "set key value", "OK");

        snprintf(cmd, sizeof(cmd), "expireat key %ld", time(NULL) + 2);
        ans = kv_ask(handler, cmd, strlen(cmd));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);
        sleep(3);

        _case_answer_string_equal(handler, "del key", "0");

        _case_answer_string_equal(handler, "set key value", "OK");
        _case_answer_string_equal(handler, "del key", "1");

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_expireat_list()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        char cmd[128];
                
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "lpush lkey a b c", "3");

        snprintf(cmd, sizeof(cmd), "expireat lkey %ld", time(NULL) + 1);
        ans = kv_ask(handler, cmd, strlen(cmd));
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        _case_answer_length_equal(handler, "lrange lkey 0 -1", 3);

        usleep((useconds_t)1.1E6);
        ans = kv_ask(handler, "lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void case_cmd_expireat_set()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        char cmd[128];
                
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "sadd skey a b c", "3");

        snprintf(cmd, sizeof(cmd), "expireat skey %ld", time(NULL) + 1);
        ans = kv_ask(handler, cmd, strlen(cmd));
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        _case_answer_length_equal(handler, "smembers skey", 3);

        usleep((useconds_t)1.1E6);
        ans = kv_ask(handler, "smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void case_cmd_expireat_without_key()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        char cmd[128];
        
        handler = kv_create(NULL);

        snprintf(cmd, sizeof(cmd), "expireat key %ld", time(NULL) + 10);
        ans = kv_ask(handler, cmd, strlen(cmd));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_expireat_invalid_cmd()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);
        ans = kv_ask(handler, "expirea key 10", strlen("expirea key 10"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_expireat_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        ans = kv_ask(handler, "expireat", strlen("expireat"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans); 
        
        ans = kv_ask(handler, "expireat key", strlen("expireat key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);

        ans = kv_ask(handler, "expireat key 10 20", strlen("expireat key 10 20"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);

        ans = kv_ask(handler, "expireat key 123456789012345678901234567890", strlen("expireat key 123456789012345678901234567890"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_expireat_multi_handler()
{
        char buf[128];
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

        snprintf(buf, sizeof(buf), "expireat keyA %ld", time(NULL)+1);
        ans = kv_ask(handlerA, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        snprintf(buf, sizeof(buf), "expireat keyB %ld", time(NULL)+3);
        ans = kv_ask(handlerB, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        snprintf(buf, sizeof(buf), "expireat keyC %ld", time(NULL)+5);
        ans = kv_ask(handlerC, buf, strlen(buf));
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

void case_cmd_expireat_stress()
{
        int i;
        kv_handler_t *handler;
        kv_answer_t *ans;
        static char buf[10000+128];
        
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "set key value", "OK");
        
        for (i = 10000; i >= 1; i--) {
                snprintf(buf, sizeof(buf), "expireat key %ld", time(NULL) + i);
                ans = kv_ask(handler, buf, strlen(buf));
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
        }

        _case_answer_string_equal(handler, "get key", "value");

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
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_get);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_list);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_set);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_expireat_stress);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
