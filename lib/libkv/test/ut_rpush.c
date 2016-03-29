#include <stdlib.h>
#include "ut_public.h"




void case_cmd_rpush_add_few_bytes()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        kv_answer_iter_t *iter;
        kv_answer_value_t *value;

        handler = kv_create(NULL);
	
   
	ans = kv_ask(handler, "rpush lkey a", strlen("rpush lkey a"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);
        
	ans = kv_ask(handler, "rpush lkey a", strlen("rpush lkey a"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "2");
        kv_answer_release(ans);

        ans = kv_ask(handler, "rpush lkey b", strlen("rpush lkey b"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "3");
        kv_answer_release(ans);

        ans = kv_ask(handler, "rpush lkey c", strlen("rpush lkey c"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "4");
        kv_answer_release(ans);

        ans = kv_ask(handler, "rpush lkey d e f g h", strlen("rpush lkey d e f g h"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "9");
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "dbsize", "1");
        
        ans = kv_ask(handler, "lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	

        iter = kv_answer_get_iter(ans, ANSWER_HEAD);
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "a");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "a");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "b");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "c");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "d");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "e");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "f");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "g");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "h");
        CU_ASSERT_PTR_EQUAL(kv_answer_next(iter), NULL);

        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_rpush_add_lot_bytes()
{
        int i;
        kv_handler_t *handler;
        kv_answer_t *ans;
        kv_answer_iter_t *iter;
        kv_answer_value_t *value;
        case_data_t *dc[1000];
        static char cmd[10000 + 128];

        handler = kv_create(NULL);

        for (i = 1; i <= 1000; i++) {
                dc[i-1] = generator_rand_str(i);
        }

        for (i = 1; i <= 1000; i++) {
                bzero(cmd, sizeof(cmd));
                strcat(cmd, "rpush lkey ");
                memcpy(cmd + strlen("rpush lkey "), dc[i-1]->ptr, dc[i-1]->ptrlen);
                ans = kv_ask(handler, cmd, dc[i-1]->ptrlen + strlen("rpush lkey "));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                kv_answer_release(ans);
        }

        ans = kv_ask(handler, "lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = kv_answer_get_iter(ans, ANSWER_HEAD);

        for (i = 1; i <= 1000; i++) {
                value = kv_answer_next(iter);
                CU_ASSERT_EQUAL(memcmp(value->ptr, dc[i-1]->ptr, dc[i-1]->ptrlen), 0);
        }
        
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        for (i = 1; i <= 1000; i++) {
                case_data_release(dc[i-1]);
        }
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_rpush_del()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);
        
        ans = kv_ask(handler, "rpush lkey 123 456", strlen("rpush lkey 123 456"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "del lkey", strlen("del lkey"));
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handler, "lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "dbsize", "0");
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_rpush_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        ans = kv_ask(handler, "rpush", strlen("rpush"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);

        ans = kv_ask(handler, "rpush lkey", strlen("rpush lkey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void case_cmd_rpush_multi_handler()
{
        int i;
        kv_answer_t *ans;
        char buf[128];
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);  

        for (i = 0; i < 10000; i++) {
                snprintf(buf, sizeof(buf), "rpush lkey%d a b c", i);
                ans = kv_ask(handlerA, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "3");
                kv_answer_release(ans);

                ans = kv_ask(handlerB, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "3");
                kv_answer_release(ans);

                ans = kv_ask(handlerC, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "3");
                kv_answer_release(ans);
        }

        _case_answer_string_equal(handlerA, "dbsize", "10000");
        _case_answer_string_equal(handlerB, "dbsize", "10000");
        _case_answer_string_equal(handlerC, "dbsize", "10000");

        for (i = 0; i < 10000; i++) {
                snprintf(buf, sizeof(buf), "lrange lkey%d 0 -1", i);
                ans = kv_ask(handlerA, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 3);
                kv_answer_iter_t *iter = kv_answer_get_iter(ans, ANSWER_TAIL);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "c");
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "b");
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "a");
                kv_answer_release_iter(iter);
                kv_answer_release(ans);
                
                ans = kv_ask(handlerB, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 3);
                iter = kv_answer_get_iter(ans, ANSWER_TAIL);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "c");
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "b");
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "a");
                kv_answer_release_iter(iter);
                kv_answer_release(ans);

                ans = kv_ask(handlerC, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                CU_ASSERT_EQUAL(kv_answer_length(ans), 3);
                iter = kv_answer_get_iter(ans, ANSWER_TAIL);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "c");
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "b");
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "a");
                kv_answer_release_iter(iter);
                kv_answer_release(ans);
        }
        
        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_rpush_stress()
{
        int i;
        char *buf, *temp;
        kv_answer_t *ans;
        case_data_t *test_data[10000];
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        buf = malloc(10000*10001+128);
        temp = malloc(10000*10001+128);
        
        for (i = 0; i < 1000; i++) {
                test_data[i] = generator_rand_str(i+1);
        }

        strcpy(buf, "rpush lkey");
        for (i = 0; i < 1000; i++) {
                strcat(buf, " ");
                strcat(buf, test_data[i]->ptr);
        }

        /** a long value */ 
        ans = kv_ask(handlerA, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1000");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1000");
        kv_answer_release(ans);
        
        ans = kv_ask(handlerC, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1000");
        kv_answer_release(ans);

        /** a long key */
        case_data_t *longkey = generator_rand_str(100000);
        sprintf(buf, "rpush %s value", longkey->ptr);
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

        case_data_release(longkey);

        _case_answer_string_equal(handlerA, "flushdb", "OK");
        _case_answer_string_equal(handlerB, "flushdb", "OK");
        _case_answer_string_equal(handlerC, "flushdb", "OK");

        for (i = 0; i < 1000; i++) {
                memcpy(buf, "rpush ", strlen("rpush "));
                memcpy(buf + strlen("rpush "), test_data[i]->ptr, test_data[i]->ptrlen);
                memcpy(buf + strlen("rpush ") + test_data[i]->ptrlen, " ", 1);
                memcpy(buf + strlen("rpush ") + test_data[i]->ptrlen + 1, test_data[i]->ptr, test_data[i]->ptrlen);
                memcpy(buf + strlen("rpush ") + test_data[i]->ptrlen + 1 + test_data[i]->ptrlen, "\0", 1);

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

        _case_answer_string_equal(handlerA, "dbsize", "1000");
        _case_answer_string_equal(handlerB, "dbsize", "1000");
        _case_answer_string_equal(handlerC, "dbsize", "1000");

        for (i = 0; i < 1000; i++) {
                memcpy(buf, "lrange ", strlen("lrange "));
                memcpy(buf + strlen("lrange "), test_data[i]->ptr, test_data[i]->ptrlen);
                memcpy(buf + strlen("lrange ") + test_data[i]->ptrlen, " 0 -1", strlen(" 0 -1")+1);
                memcpy(buf + strlen("lrange ") + test_data[i]->ptrlen + strlen(" 0 -1"), "\0", 1);

                ans = kv_ask(handlerA, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                memcpy(temp, test_data[i]->ptr, test_data[i]->ptrlen);
                memcpy(temp + test_data[i]->ptrlen, "\0", 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), temp);
                kv_answer_release(ans);

                ans = kv_ask(handlerB, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                memcpy(temp, test_data[i]->ptr, test_data[i]->ptrlen);
                memcpy(temp + test_data[i]->ptrlen, "\0", 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), temp);
                kv_answer_release(ans);

                ans = kv_ask(handlerC, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                memcpy(temp, test_data[i]->ptr, test_data[i]->ptrlen);
                memcpy(temp + test_data[i]->ptrlen, "\0", 1);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), temp);
                kv_answer_release(ans);
        }

        free(buf);
        free(temp);
        for (i = 0; i < 1000; i++) {
                case_data_release(test_data[i]);
        }
	
	ans = kv_ask(handlerA, "rpush lkey abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", strlen("rpush lkey abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
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
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_add_few_bytes);
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_add_lot_bytes);
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_del);
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_rpush_stress);
        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
