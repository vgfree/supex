#include "ut_public.h"


void case_cmd_hget()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "hget hkey field", strlen("hget hkey field"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "hset hkey field value", "1");
        ans = kv_ask(handler, "hget hkey field", strlen("hget hkey field"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "value");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "del hkey", "1");
        _case_answer_string_equal(handler, "hset hkey f1 v1", "1");
        _case_answer_string_equal(handler, "hset hkey f2 v2", "1");
        _case_answer_string_equal(handler, "hset hkey f3 v3", "1");

        ans = kv_ask(handler, "hget hkey f1", strlen("hget hkey f1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "v1");
        kv_answer_release(ans);

        ans = kv_ask(handler, "hget hkey f2", strlen("hget hkey f2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "v2");
        kv_answer_release(ans);

        ans = kv_ask(handler, "hget hkey f3", strlen("hget hkey f3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "v3");
        kv_answer_release(ans);

        ans = kv_ask(handler, "hget hkey f4", strlen("hget hkey f4"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "set key value", "OK");
        ans = kv_ask(handler, "hget key field", strlen("hget key field"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);
        
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hget_invalid_cmd()
{
        kv_handler_t *handler;
        kv_answer_t *ans;


        handler = kv_create(NULL);
        CU_ASSERT(handler != NULL);

        ans = kv_ask(handler, "hge key f", strlen("hge key f"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        kv_answer_release(ans);

        ans = kv_ask(handler, "hgete key f", strlen("hgete key f"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hget_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;


        handler = kv_create(NULL);
        CU_ASSERT(handler != NULL);

        ans = kv_ask(handler, "hget", strlen("hget"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "hget key", strlen("hget key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);

        ans = kv_ask(handler, "hget key f v", strlen("hget key f v"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hget_multi_handler()
{
        kv_handler_t *handlerA, *handlerB, *handlerC;
        kv_answer_t *ans;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        _case_answer_string_equal(handlerA, "hset hkey field valueA", "1");
        _case_answer_string_equal(handlerB, "hset hkey field valueB", "1");
        _case_answer_string_equal(handlerC, "hset hkey field valueC", "1");

        ans = kv_ask(handlerA, "hget hkey field", strlen("hget hkey field"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "valueA");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "hget hkey field", strlen("hget hkey field"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "valueB");
        kv_answer_release(ans);
        
        ans = kv_ask(handlerC, "hget hkey field", strlen("hget hkey field"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "valueC");
        kv_answer_release(ans);

        
        _case_answer_string_equal(handlerA, "del hkey", "1");
        _case_answer_string_equal(handlerC, "del hkey", "1");

        ans = kv_ask(handlerA, "hget hkey field", strlen("hget hkey field"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "hget hkey field", strlen("hget hkey field"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "valueB");
        kv_answer_release(ans);
        
        ans = kv_ask(handlerC, "hget hkey field", strlen("hget hkey field"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
        kv_answer_release(ans);


        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hget_stress()
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

        for (i = 0; i < 64-32; i++) {
                snprintf(cmd, sizeof(cmd), "hset hkey f%d %s", i+1, test_data[i]->ptr);
                _case_answer_string_equal(handlerA, cmd, "1");
                _case_answer_string_equal(handlerB, cmd, "1");
                _case_answer_string_equal(handlerC, cmd, "1");

                snprintf(cmd, sizeof(cmd), "hget hkey f%d", i+1);
                ans = kv_ask(handlerA, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), test_data[i]->ptr);
                kv_answer_release(ans);

                ans = kv_ask(handlerB, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), test_data[i]->ptr);
                kv_answer_release(ans);
                                
                ans = kv_ask(handlerC, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), test_data[i]->ptr);
                kv_answer_release(ans);
        }

        _case_answer_string_equal(handlerA, "del hkey", "1");
        _case_answer_string_equal(handlerB, "del hkey", "1");
        _case_answer_string_equal(handlerC, "del hkey", "1");
        
        for (i = 0; i < 2*1024; i++) {
                snprintf(cmd, sizeof(cmd), "hset hkey f%d %s", i+1, test_data[i]->ptr);
                _case_answer_string_equal(handlerA, cmd, "1");
                _case_answer_string_equal(handlerB, cmd, "1");
                _case_answer_string_equal(handlerC, cmd, "1");

                snprintf(cmd, sizeof(cmd), "hget hkey f%d", i+1);
                ans = kv_ask(handlerA, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), test_data[i]->ptr);
                kv_answer_release(ans);

                ans = kv_ask(handlerB, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), test_data[i]->ptr);
                kv_answer_release(ans);
                                
                ans = kv_ask(handlerC, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), test_data[i]->ptr);
                kv_answer_release(ans);
        }

        for (i = 0; i < 1024; i++) {
                snprintf(cmd, sizeof(cmd), "hset hlkey f%d %s\n", i+1, test_data[i]->ptr);
                _case_answer_string_equal(handlerA, cmd, "1");
                _case_answer_string_equal(handlerB, cmd, "1");
                _case_answer_string_equal(handlerC, cmd, "1");

                snprintf(cmd, sizeof(cmd), "hget hkey f%d", i+1);
                ans = kv_ask(handlerA, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), test_data[i]->ptr);
                kv_answer_release(ans);

                ans = kv_ask(handlerB, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), test_data[i]->ptr);
                kv_answer_release(ans);
                                
                ans = kv_ask(handlerC, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), test_data[i]->ptr);
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
        UTIL_ADD_CASE_SHORT(case_cmd_hget);
        UTIL_ADD_CASE_SHORT(case_cmd_hget_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_hget_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_hget_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_hget_stress);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
