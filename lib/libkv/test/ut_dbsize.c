#include <unistd.h>
#include "ut_public.h"



void case_cmd_dbsize()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        
        handler = kv_create(NULL);

        ans = kv_ask(handler, "dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "set key value", "OK");

        ans = kv_ask(handler, "dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "lpush lkey a b c", "3");

        ans = kv_ask(handler, "dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "2");
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_dbsize_expire()
{
        int i;
        char buf[128];
        kv_handler_t *handler;
        kv_answer_t *ans;
          
        handler = kv_create(NULL);

        for (i = 0; i < 100000; i++) {
                snprintf(buf, sizeof(buf), "set key%d value", i);
                _case_answer_string_equal(handler, buf, "OK");
                snprintf(buf, sizeof(buf), "expire key%d 2", i);
                _case_answer_string_equal(handler, buf, "1");
        }
        _case_answer_string_equal(handler, "dbsize", "100000");

        usleep((useconds_t)2.1*1E6);
        _case_answer_string_equal(handler, "dbsize", "0");
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_dbsize_multi_handler()
{
        kv_handler_t *handlerA, *handlerB, *handlerC;
        kv_answer_t *ans;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        /** All db size is zero after created */
        _case_answer_length_equal(handlerA, "dbsize", 1);
        _case_answer_string_equal(handlerA, "dbsize", "0");
        
        _case_answer_length_equal(handlerB, "dbsize", 1);
        _case_answer_string_equal(handlerB, "dbsize", "0");

        _case_answer_length_equal(handlerC, "dbsize", 1);
        _case_answer_string_equal(handlerC, "dbsize", "0");
        
        /** handlerA 5 keys */
        _case_answer_string_equal(handlerA, "set keyA valueA", "OK");
        _case_answer_string_equal(handlerA, "lpush lkeyA a b c", "3");
        _case_answer_string_equal(handlerA, "sadd skeyA d e f", "3");

        /** handlerB 1 key */
        _case_answer_string_equal(handlerB, "set keyB valueB", "OK");

        _case_answer_string_equal(handlerA, "dbsize", "3");
        _case_answer_string_equal(handlerB, "dbsize", "1");
        _case_answer_string_equal(handlerC, "dbsize", "0");

        /** flushdb */
        _case_answer_string_equal(handlerA, "flushdb", "OK");
        _case_answer_string_equal(handlerA, "dbsize", "0");
        _case_answer_string_equal(handlerB, "dbsize", "1");
        _case_answer_string_equal(handlerC, "dbsize", "0");

        _case_answer_string_equal(handlerA, "flushdb", "OK");
        _case_answer_string_equal(handlerB, "flushdb", "OK");
        _case_answer_string_equal(handlerC, "flushdb", "OK");
        _case_answer_string_equal(handlerA, "dbsize", "0");
        _case_answer_string_equal(handlerB, "dbsize", "0");
        _case_answer_string_equal(handlerC, "dbsize", "0");
        
        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);

        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_dbsize_stress()
{
        int i;
        char buf[10000+128];
        kv_handler_t *handlerA, *handlerB, *handlerC;
        kv_answer_t *ans;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);
        
        for (i = 1; i <= 30000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d %d", i, i);
                _case_answer_string_equal(handlerA, buf, "OK");
        }

        for (i = 1; i <= 20000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d %d", i, i);
                _case_answer_string_equal(handlerB, buf, "OK");
        }

        for (i = 1; i <= 10000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d %d", i, i);
                _case_answer_string_equal(handlerC, buf, "OK");
        }
        
        ans = kv_ask(handlerA, "dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "30000");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "20000");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "10000");
        kv_answer_release(ans);

        for (i = 1; i <= 10000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "lpush lkey%d %d", i, i);
                _case_answer_string_equal(handlerA, buf, "1");
        }
        
        ans = kv_ask(handlerA, "dbsize", strlen("dbsize"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "40000");
        kv_answer_release(ans);

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();

        UTIL_ADD_CASE_SHORT(case_cmd_dbsize);
        UTIL_ADD_CASE_SHORT(case_cmd_dbsize_expire);
        UTIL_ADD_CASE_SHORT(case_cmd_dbsize_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_dbsize_stress);

        UTIL_RUN();
        UTIL_UNINIT();
  
        return 0;
}
#endif
