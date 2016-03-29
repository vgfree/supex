#include "ut_public.h"







void case_cmd_scard()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "sadd skey 11", "1");
        ans = kv_ask(handler, "scard skey", strlen("scard skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "sadd skey 11", "0");

        _case_answer_string_equal(handler, "sadd skey 22", "1");
        ans = kv_ask(handler, "scard skey", strlen("scard skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "2");
        kv_answer_release(ans);
        
        _case_answer_string_equal(handler, "sadd skey 33", "1");
        ans = kv_ask(handler, "scard skey", strlen("scard skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "3");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "del skey", "1");
        ans = kv_ask(handler, "scard skey", strlen("scard skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        /** string */
        _case_answer_string_equal(handler, "sadd skey aa", "1");
        ans = kv_ask(handler, "scard skey", strlen("scard skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "sadd skey aa", "0");

        _case_answer_string_equal(handler, "sadd skey bb", "1");
        ans = kv_ask(handler, "scard skey", strlen("scard skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "2");
        kv_answer_release(ans);
        
        _case_answer_string_equal(handler, "sadd skey cc", "1");
        ans = kv_ask(handler, "scard skey", strlen("scard skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "3");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "del skey", "1");
        ans = kv_ask(handler, "scard skey", strlen("scard skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);
      
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);  
}

void case_cmd_scard_invalid_cmd()
{
        kv_handler_t *handler;
        kv_answer_t *ans;


        handler = kv_create(NULL);
        CU_ASSERT(handler != NULL);

        ans = kv_ask(handler, "scar skey", strlen("scar skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        kv_answer_release(ans);

        ans = kv_ask(handler, "scad skey", strlen("scad skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);  
}

void case_cmd_scard_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;


        handler = kv_create(NULL);
        CU_ASSERT(handler != NULL);

        ans = kv_ask(handler, "scard", strlen("scard"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);

        ans = kv_ask(handler, "scard skey k", strlen("scard skey k"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);  
}

void case_cmd_scard_invalid_type()
{
        kv_handler_t *handler;
        kv_answer_t *ans;


        handler = kv_create(NULL);
        CU_ASSERT(handler != NULL);

        _case_answer_string_equal(handler, "set key value", "OK");
        ans = kv_ask(handler, "scard key", strlen("scard key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "lpush lkey v1 v2", "2");
        ans = kv_ask(handler, "scard lkey", strlen("scard lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "hset hkey f v", "1");
        ans = kv_ask(handler, "scard hkey", strlen("scard hkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0); 
}

void case_cmd_scard_db()
{
        int i;
        kv_handler_t *handler;
        kv_answer_t *ans;
        static char buf[1024];

        handler = kv_create(NULL);
        for (i = 0; i < 16; i++) {
                snprintf(buf, sizeof(buf), "select %d", i);
                _case_answer_string_equal(handler, buf, "OK");

                snprintf(buf, sizeof(buf), "sadd skey%d %d", i+1);
                _case_answer_string_equal(handler, buf, "1");

                snprintf(buf, sizeof(buf), "scard skey%d", i+1);
                ans = kv_ask(handler, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
        }
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);  
}

void case_cmd_scard_multi_handler()
{
        int i;
        kv_handler_t *handlerA, *handlerB, *handlerC;
        kv_answer_t *ans;
        static char buf[1024], buf_ret[24];

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);   

        for (i = 1; i <= 100; i++) {
                snprintf(buf, sizeof(buf), "sadd handlerA_skey %d", i);
                _case_answer_string_equal(handlerA, buf, "1");

                snprintf(buf_ret, sizeof(buf_ret), "%d", i);
                snprintf(buf, sizeof(buf), "scard handlerA_skey");
                ans = kv_ask(handlerA, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), buf_ret);
                kv_answer_release(ans);
        }

        for (i = 1; i <= 100; i++) {
                snprintf(buf, sizeof(buf), "sadd handlerB_skey %d", i);
                _case_answer_string_equal(handlerB, buf, "1");

                snprintf(buf_ret, sizeof(buf_ret), "%d", i);
                snprintf(buf, sizeof(buf), "scard handlerB_skey");
                ans = kv_ask(handlerB, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), buf_ret);
                kv_answer_release(ans);
        }

        for (i = 1; i <= 100; i++) {
                snprintf(buf, sizeof(buf), "sadd handlerC_skey %d", i);
                _case_answer_string_equal(handlerC, buf, "1");

                snprintf(buf_ret, sizeof(buf_ret), "%d", i);
                snprintf(buf, sizeof(buf), "scard handlerC_skey");
                ans = kv_ask(handlerC, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), buf_ret);
                kv_answer_release(ans);
        }

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);  
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_scard);
        UTIL_ADD_CASE_SHORT(case_cmd_scard_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_scard_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_scard_invalid_type);
        UTIL_ADD_CASE_SHORT(case_cmd_scard_db);
        UTIL_ADD_CASE_SHORT(case_cmd_scard_multi_handler);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
