#include "ut_public.h"



void case_cmd_flushdb()
{
        /*
         * flush empty db
         * flush one key db
         * flush 5000 string keys and 5000 lpush keys
         */
        kv_handler_t *handler;
        kv_answer_t *ans;
        static char buf[10000+128];
        int i;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "dbsize", "0");

        /* flush one key */
        _case_answer_string_equal(handler, "set key value", "OK");

        ans = kv_ask(handler, "flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "dbsize", "0");
        
        /* flush list key */
        _case_answer_string_equal(handler, "lpush lkey a b c", "3");

        ans = kv_ask(handler, "flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "dbsize", "0");

        /* 5000 string keys and 5000 list keys */
        for (i = 1; i <= 5000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d %d", i, i);

                _case_answer_string_equal(handler, buf, "OK");

                bytes = snprintf(buf, sizeof(buf), "lpush lkey%d %d", i, i);
                _case_answer_string_equal(handler, buf, "1");
        }

        _case_answer_string_equal(handler, "dbsize", "10000");
        
        ans = kv_ask(handler, "flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "dbsize", "0");
        
        kv_destroy(handler);
}

void case_cmd_flushdb_multi_handler()
{
        int i;
        char buf[10000+128];
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);
        
        for (i = 1; i <= 5000; i++) {
                snprintf(buf, sizeof(buf), "set key%d value%d", i);
                _case_answer_string_equal(handlerA, buf, "OK");
                _case_answer_string_equal(handlerB, buf, "OK");

                snprintf(buf, sizeof(buf), "lpush lkey%d a b c", i);
                _case_answer_string_equal(handlerA, buf, "3");
                _case_answer_string_equal(handlerB, buf, "3");

                snprintf(buf, sizeof(buf), "sadd skey%d a b c", i);
                _case_answer_string_equal(handlerA, buf, "3");
                _case_answer_string_equal(handlerC, buf, "3");
        }

        ans = kv_ask(handlerA, "flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);
        
        ans = kv_ask(handlerB, "flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);
        
        ans = kv_ask(handlerC, "flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "dbsize", "0");
        _case_answer_string_equal(handlerB, "dbsize", "0");
        _case_answer_string_equal(handlerC, "dbsize", "0");
        
        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}

void case_cmd_flushdb_stress()
{
        int i;
        char buf[10000+128];
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);
        
        for (i = 1; i <= 100000; i++) {
                snprintf(buf, sizeof(buf), "set key%d value%d", i);
                _case_answer_string_equal(handlerA, buf, "OK");
                _case_answer_string_equal(handlerB, buf, "OK");
                _case_answer_string_equal(handlerC, buf, "OK"); 

                snprintf(buf, sizeof(buf), "lpush lkey%d a b c", i);
                _case_answer_string_equal(handlerA, buf, "3");
                _case_answer_string_equal(handlerB, buf, "3");
                _case_answer_string_equal(handlerC, buf, "3");

                snprintf(buf, sizeof(buf), "sadd skey%d a b c", i);
                _case_answer_string_equal(handlerA, buf, "3");
                _case_answer_string_equal(handlerB, buf, "3");
                _case_answer_string_equal(handlerC, buf, "3");
        }

        ans = kv_ask(handlerA, "flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);
        
        ans = kv_ask(handlerB, "flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);
        
        ans = kv_ask(handlerC, "flushdb", strlen("flushdb"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "dbsize", "0");
        _case_answer_string_equal(handlerB, "dbsize", "0");
        _case_answer_string_equal(handlerC, "dbsize", "0");

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        
        UTIL_ADD_CASE_SHORT(case_cmd_flushdb);
        UTIL_ADD_CASE_SHORT(case_cmd_flushdb_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_flushdb_stress); 
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
