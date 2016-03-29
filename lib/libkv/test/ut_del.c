#include "ut_public.h"





void case_cmd_del()
{
        /*
         * delete no_exist key
         * delete a exist key
         * delete multiple exist keys
         * delete multiple keys with some no_exist
         */
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "del key_noexist", strlen("del key_noexist"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "set key value", "OK");
        
        ans = kv_ask(handler, "del key", strlen("del key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "set key1 value1", "OK");
        _case_answer_string_equal(handler, "set key2 value2", "OK");
        _case_answer_string_equal(handler, "set key3 value3", "OK");
        
        ans = kv_ask(handler, "del key1 key2 key3", strlen("del key1 key2 key3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "3");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "dbsize", "0");

        _case_answer_string_equal(handler, "set key4 value4", "OK");
        _case_answer_string_equal(handler, "set key5 value5", "OK");
        _case_answer_string_equal(handler, "set key6 value6", "OK");

        ans = kv_ask(handler, "del key4 key5 key7", strlen("del key4 key5 key7"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "2");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "dbsize", "1");

        kv_destroy(handler);
}

void case_cmd_del_multi_handler()
{
        int i;
        char buf[10000+128];
        kv_handler_t *handlerA, *handlerB, *handlerC;

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

        _case_answer_string_equal(handlerA, "dbsize", "30000");
        _case_answer_string_equal(handlerB, "dbsize", "20000");
        _case_answer_string_equal(handlerC, "dbsize", "10000");

        for (i = 1; i <= 30000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "del key%d", i);
                _case_answer_string_equal(handlerA, buf, "1");
        }
           
        for (i = 1; i <= 20000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "del key%d", i);
                _case_answer_string_equal(handlerB, buf, "1");
        }
   
        for (i = 1; i <= 10000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "del key%d", i);
                _case_answer_string_equal(handlerC, buf, "1");
        }

        _case_answer_string_equal(handlerA, "dbsize", "0");
        _case_answer_string_equal(handlerB, "dbsize", "0");
        _case_answer_string_equal(handlerC, "dbsize", "0");

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}

void case_cmd_del_stress()
{
        int i;
        char buf[10000+128];
        kv_answer_t *ans;
        static char all_keys_buf[30000*30001/2+128];
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL); 

        /** del one key every time */
        for (i = 1; i <= 30000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d value%d", i);
                _case_answer_string_equal(handlerA, buf, "OK");
                _case_answer_string_equal(handlerB, buf, "OK");
                _case_answer_string_equal(handlerC, buf, "OK");
        }

        for (i = 1; i <= 30000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "del key%d", i);
                ans = kv_ask(handlerA, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerB, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerC, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
        }

        _case_answer_string_equal(handlerA, "dbsize", "0");
        _case_answer_string_equal(handlerB, "dbsize", "0");
        _case_answer_string_equal(handlerC, "dbsize", "0");

        /** del all keys at the same time */
        for (i = 1; i <= 30000; i++) {
                int bytes = snprintf(buf, sizeof(buf), "set key%d value%d", i);
                _case_answer_string_equal(handlerA, buf, "OK");
                _case_answer_string_equal(handlerB, buf, "OK");
                _case_answer_string_equal(handlerC, buf, "OK");
        }

        strcpy(all_keys_buf, "del ");
        for (i = 1; i <= 30000; i++) {
                int bytes = snprintf(buf, sizeof(buf), " key%d", i);
                strcat(all_keys_buf, buf);
        }
        
        ans = kv_ask(handlerA, all_keys_buf, strlen(all_keys_buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "30000");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, all_keys_buf, strlen(all_keys_buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "30000");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, all_keys_buf, strlen(all_keys_buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "30000");
        kv_answer_release(ans);
        
        _case_answer_string_equal(handlerA, "dbsize", "0");
        _case_answer_string_equal(handlerB, "dbsize", "0");
        _case_answer_string_equal(handlerC, "dbsize", "0");


        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
        
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_cmd_del);
        UTIL_ADD_CASE_SHORT(case_cmd_del_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_del_stress);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
