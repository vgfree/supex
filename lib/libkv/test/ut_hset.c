#include "ut_public.h"



void case_cmd_hset()
{
        kv_handler_t *handler;
        kv_answer_t *ans;


        handler = kv_create(NULL);
        CU_ASSERT(handler != NULL);

        ans = kv_ask(handler, "hset key f v", strlen("hset key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);
        _case_answer_string_equal(handler, "hget key f", "v");
        
        ans = kv_ask(handler, "hset key f v", strlen("hset key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);
        _case_answer_string_equal(handler, "hget key f", "v");

        ans = kv_ask(handler, "hset key f1 v1", strlen("hset key f1 v1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);
         _case_answer_string_equal(handler, "hget key f1", "v1");

        ans = kv_ask(handler, "hset key f1 v1", strlen("hset key f1 v1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);
        _case_answer_string_equal(handler, "hget key f1", "v1");
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hset_invalid_cmd()
{
        kv_handler_t *handler;
        kv_answer_t *ans;


        handler = kv_create(NULL);
        CU_ASSERT(handler != NULL);

        ans = kv_ask(handler, "hse key f v", strlen("hse key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        kv_answer_release(ans);

        ans = kv_ask(handler, "hsete key f v", strlen("hsete key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hset_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;


        handler = kv_create(NULL);
        CU_ASSERT(handler != NULL);

        ans = kv_ask(handler, "hset", strlen("hset"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "hset key", strlen("hset key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);

        ans = kv_ask(handler, "hset key f", strlen("hset key f"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);

        ans = kv_ask(handler, "hset key f v v1", strlen("hset key f v v1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hset_multi_handler()
{
        kv_handler_t *handlerA, *handlerB, *handlerC;
        kv_answer_t *ans;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        /** reply 1 */
        ans = kv_ask(handlerA, "hset key f v", strlen("hset key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);
        _case_answer_string_equal(handlerA, "hget key f", "v");
        
        ans = kv_ask(handlerB, "hset key f v", strlen("hset key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);
        _case_answer_string_equal(handlerB, "hget key f", "v"); 

        ans = kv_ask(handlerC, "hset key f v", strlen("hset key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);
        _case_answer_string_equal(handlerC, "hget key f", "v"); 

        /** reply 0 */
        ans = kv_ask(handlerA, "hset key f v", strlen("hset key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "hset key f v", strlen("hset key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "hset key f v", strlen("hset key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        /** reply->1 0 1 */
        _case_answer_string_equal(handlerA, "del key", "1");
        ans = kv_ask(handlerA, "hset key f v", strlen("hset key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "hset key f v", strlen("hset key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        _case_answer_string_equal(handlerC, "del key", "1");
        ans = kv_ask(handlerC, "hset key f v", strlen("hset key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        
        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hset_stress()
{
        int i;
        kv_handler_t *handlerA, *handlerB, *handlerC;
        kv_answer_t *ans;
        case_data_t *test_data[1024*2];
        static char cmd[1024*2048+1024];
   
        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        for (i = 0; i < 2*1024; i++) {
                test_data[i] = generator_rand_str(i+1);
        }

        for (i = 0; i < 64; i++) {
                snprintf(cmd, sizeof(cmd), "hset hkey f%d %s", i+1, test_data[i]->ptr);
                ans = kv_ask(handlerA, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerB, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerC, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
        }

        _case_answer_string_equal(handlerA, "del hkey", "1");
        _case_answer_string_equal(handlerB, "del hkey", "1");
        _case_answer_string_equal(handlerC, "del hkey", "1");
        
        for (i = 0; i < 2*1024; i++) {
                snprintf(cmd, sizeof(cmd), "hset hkey f%d %s", i+1, test_data[i]->ptr);
                ans = kv_ask(handlerA, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerB, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerC, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
        }

        for (i = 0; i < 1024; i++) {
                snprintf(cmd, sizeof(cmd), "hset hlkey f%d %s\n", i+1, test_data[i]->ptr);
                ans = kv_ask(handlerA, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerB, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);

                ans = kv_ask(handlerC, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
        }
                
        for (i = 0; i < 2*1024; i++) {
                case_data_release(test_data[i]);
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
        UTIL_ADD_CASE_SHORT(case_cmd_hset);
        UTIL_ADD_CASE_SHORT(case_cmd_hset_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_hset_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_hset_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_hset_stress);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
