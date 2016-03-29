#include "ut_public.h"



void case_cmd_srandmember()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        ans = kv_ask(handler, "srandmember skey", strlen("srandmember skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "sadd skey a b c", "3");
        ans = kv_ask(handler, "srandmember skey", strlen("srandmember skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT(strcmp(kv_answer_value_to_string(kv_answer_first_value(ans)), "a") == 0
                  || strcmp(kv_answer_value_to_string(kv_answer_first_value(ans)), "b") == 0
                  || strcmp(kv_answer_value_to_string(kv_answer_first_value(ans)), "c") == 0);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmember skey 3", strlen("srandmember skey 3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 3);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmember skey 1", strlen("srandmember skey 1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "srandmember skey 2", strlen("srandmember skey 2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 2);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmember skey 4", strlen("srandmember skey 4"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 3);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmember skey -1", strlen("srandmember skey -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT(strcmp(kv_answer_value_to_string(kv_answer_first_value(ans)), "a") == 0
                  || strcmp(kv_answer_value_to_string(kv_answer_first_value(ans)), "b") == 0
                  || strcmp(kv_answer_value_to_string(kv_answer_first_value(ans)), "c") == 0);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmember skey -2", strlen("srandmember skey -2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 2);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmember skey -3", strlen("srandmember skey -3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 3);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmember skey -4", strlen("srandmember skey -4"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 4);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "sadd _skey a b c d", "4");
        ans = kv_ask(handler, "srandmember _skey 5", strlen("srandmember _skey 5"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 4);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_srandmember_invalid_cmd()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "sadd skey a b c", "3");
        ans = kv_ask(handler, "srandmember skey", strlen("srandmember skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmeber skey", strlen("srandmeber skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmembe skey", strlen("srandmembe skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}


void case_cmd_srandmember_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "sadd skey a b c", "3");
        ans = kv_ask(handler, "srandmember", strlen("srandmember"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmember", strlen("srandmember"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmember skey xx", strlen("srandmember skey xx"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_OUT_OF_RANGE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        ans = kv_ask(handler, "srandmember skey 1 3", strlen("srandmember skey 1 3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_SYNTAX);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_srandmember_invalid_type()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);
        
        _case_answer_string_equal(handler, "set key value", "OK");
        ans = kv_ask(handler, "srandmember key", strlen("srandmember key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "lpush lkey value", "1");
        ans = kv_ask(handler, "srandmember lkey", strlen("srandmember lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "hset hkey f v", "1");
        ans = kv_ask(handler, "srandmember hkey", strlen("srandmember hkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_srandmember_db()
{
        int i;
        kv_handler_t *handler;
        kv_answer_t *ans;
        static char buf[1024];

        handler = kv_create(NULL);

        int incr = 1;
        for (i = 0; i < 16; i++) {
                snprintf(buf, sizeof(buf), "select %d", i);
                _case_answer_string_equal(handler, buf, "OK");

                snprintf(buf, sizeof(buf), "sadd skey%d %d", i+1);
                _case_answer_string_equal(handler, buf, "1");

                snprintf(buf, sizeof(buf), "srandmember skey%d", i+1);
                ans = kv_ask(handler, buf, strlen(buf));
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                kv_answer_release(ans);

                snprintf(buf, sizeof(buf), "srandmember skey%d 1", i+1);
                ans = kv_ask(handler, buf, strlen(buf));
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                kv_answer_release(ans);

                snprintf(buf, sizeof(buf), "srandmember skey%d 2", i+1);
                ans = kv_ask(handler, buf, strlen(buf));
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                kv_answer_release(ans);
        }

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0); 
}

void case_cmd_srandmember_multi_handler()
{
        kv_handler_t *handlerA;
        kv_handler_t *handlerB;
        kv_handler_t *handlerC;
        kv_answer_t *ans;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        _case_answer_string_equal(handlerA, "sadd skey a", "1");
        _case_answer_string_equal(handlerB, "sadd skey a b", "2");
        _case_answer_string_equal(handlerC, "sadd skey a b c", "3");

        ans = kv_ask(handlerA, "srandmember skey 4", strlen("srandmember skey 4"));
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        kv_answer_release(ans);
        
        ans = kv_ask(handlerB, "srandmember skey 4", strlen("srandmember skey 4"));
        CU_ASSERT_EQUAL(kv_answer_length(ans), 2);
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "srandmember skey 4", strlen("srandmember skey 4"));
        CU_ASSERT_EQUAL(kv_answer_length(ans), 3);
        kv_answer_release(ans);


        ans = kv_ask(handlerA, "srandmember skey", strlen("srandmember skey"));
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        kv_answer_release(ans);
        
        ans = kv_ask(handlerB, "srandmember skey 1", strlen("srandmember skey 1"));
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "srandmember skey 3", strlen("srandmember skey 3"));
        CU_ASSERT_EQUAL(kv_answer_length(ans), 3);
        kv_answer_release(ans);
        

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember);
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember_db);
        UTIL_ADD_CASE_SHORT(case_cmd_srandmember_multi_handler);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
