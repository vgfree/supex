#include "ut_public.h"




void cmd_case_smembers_without_members()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        ans = kv_ask(handler, "smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(kv_answer_first_value(ans), NULL);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void cmd_case_smembers_with_members()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        kv_answer_iter_t *iter;
        kv_answer_value_t *value;
        
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "sadd skey a b c", "3");

        ans = kv_ask(handler, "smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 3);
        iter = kv_answer_get_iter(ans, ANSWER_HEAD);
        while((value = kv_answer_next(iter)) != NULL) {
                CU_ASSERT(!strcmp(kv_answer_value_to_string(value), "a") ||
                          !strcmp(kv_answer_value_to_string(value), "b") ||
                          !strcmp(kv_answer_value_to_string(value), "c"));
        }
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void cmd_case_smembers_invalid_cmd()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        kv_answer_iter_t *iter;
        
        handler = kv_create(NULL);

        ans = kv_ask(handler, "smemberss skey", strlen("smemberss skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        CU_ASSERT_PTR_EQUAL(kv_answer_first_value(ans), NULL);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void cmd_case_smembers_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        kv_answer_iter_t *iter;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "smembers", strlen("smembers"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_first_value(ans), NULL);
        kv_answer_release(ans);

        kv_destroy(handler); 
}

void cmd_case_smembers_invalid_type()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        kv_answer_iter_t *iter;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "set key value", strlen("set key value"));
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "smembers key", strlen("smembers key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        CU_ASSERT_PTR_EQUAL(kv_answer_first_value(ans), NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler); 
}

void cmd_case_smembers_multi_handler()
{
        int i;
        kv_answer_t *ans;
        char short_buf[32];
        static char long_buf[10000*10001/2 + 10];
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL); 

        strcpy(long_buf, "sadd skey ");
        for (i = 1; i <= 10000; i++) {
                snprintf(short_buf, sizeof(short_buf), "%d ", i);
                strcat(long_buf, short_buf);
        }

        ans = kv_ask(handlerA, long_buf, strlen(long_buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "10000");
        kv_answer_release(ans);
        
        ans = kv_ask(handlerB, long_buf, strlen(long_buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "10000");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, long_buf, strlen(long_buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "10000");
        kv_answer_release(ans);

        ans = kv_ask(handlerA, "smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(kv_answer_length(ans), 10000);
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(kv_answer_length(ans), 10000);
        kv_answer_release(ans);
        
        ans = kv_ask(handlerC, "smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(kv_answer_length(ans), 10000);
        kv_answer_release(ans);
        
        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}

void cmd_case_smembers_stress()
{
        int i;
        kv_handler_t *handler;
        kv_answer_t *ans;
        char short_buf[32];
        static char long_buf[10000*10001/2 + 10];

        handler = kv_create(NULL);

        strcpy(long_buf, "sadd skey ");
        for (i = 1; i <= 10000; i++) {
                snprintf(short_buf, sizeof(short_buf), "%d ", i);
                strcat(long_buf, short_buf);
        }

        _case_answer_string_equal(handler, long_buf, "10000");

        ans = kv_ask(handler, "smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 10000);
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_without_members);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_with_members);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_cmd);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_param);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_invalid_type);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_multi_handler);
        UTIL_ADD_CASE_SHORT(cmd_case_smembers_stress);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
