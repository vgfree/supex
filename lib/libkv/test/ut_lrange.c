#include "ut_public.h"




void case_cmd_lrange_without_found()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        kv_answer_iter_t *iter;
        kv_answer_value_t *value;
        

        handler = kv_create(NULL);

        ans = kv_ask(handler, "lrange mylist 0 1", strlen("lrange mylist 0 1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(kv_answer_first_value(ans), NULL);
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lrange_found()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        kv_answer_iter_t *iter;
        kv_answer_value_t *value;
        
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "lpush mylist a b c", "3");

        ans = kv_ask(handler, "lrange mylist 0 0", strlen("lrange mylist 0 0"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = kv_answer_get_iter(ans, ANSWER_HEAD);
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "c");
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), NULL);
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        ans = kv_ask(handler, "lrange mylist 0 2", strlen("lrange mylist 0 2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = kv_answer_get_iter(ans, ANSWER_HEAD);
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "c");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "b");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "a");
        value = kv_answer_next(iter);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(value), NULL);
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        ans = kv_ask(handler, "lrange mylist 0 3", strlen("lrange mylist 0 3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = kv_answer_get_iter(ans, ANSWER_HEAD);
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "c");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "b");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "a");
        value = kv_answer_next(iter);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(value), NULL);
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        ans = kv_ask(handler, "lrange mylist 0 -1", strlen("lrange mylist 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = kv_answer_get_iter(ans, ANSWER_HEAD);
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "c");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "b");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "a");
        value = kv_answer_next(iter);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(value), NULL);
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        ans = kv_ask(handler, "lrange mylist -1 -1", strlen("lrange mylist -1 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = kv_answer_get_iter(ans, ANSWER_HEAD);
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "a");
        value = kv_answer_next(iter);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(value), NULL);
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        ans = kv_ask(handler, "lrange mylist -1 1", strlen("lrange mylist -1 1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        ans = kv_ask(handler, "lrange mylist 1 -1", strlen("lrange mylist 1 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        iter = kv_answer_get_iter(ans, ANSWER_HEAD);
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "b");
        value = kv_answer_next(iter);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), "a");
        value = kv_answer_next(iter);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(value), NULL);
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lrange_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);
        ans = kv_ask(handler, "lrange lkey 0", strlen("lrange lkey 0"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_first_value(ans), NULL);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "lrange lkey 0 -1 2", strlen("lrange lkey 0 -1 2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_first_value(ans), NULL);
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lrange_multi_db()
{
	kv_handler_t *handler;
	kv_answer_t *ans;
	
	handler = kv_create(NULL);
	_case_answer_string_equal(handler, "select 2", "OK");
	
        ans = kv_ask(handler, "lrange lkey 0 1", strlen("lrange lkey 0 1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(kv_answer_first_value(ans), NULL);
        kv_answer_release(ans);

	_case_answer_string_equal(handler, "select 3", "OK");
	
        ans = kv_ask(handler, "lrange lkey 0 1", strlen("lrange lkey 0 1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(kv_answer_first_value(ans), NULL);
        kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}
void case_cmd_lrange_multi_handler()
{
        int i;
        case_data_t *test_data[1000];
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;
        char buf[1000*1001];

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL); 

        for (i = 0; i < 1000; i++) {
                test_data[i] = generator_rand_str(i+1);
        }

        strcpy(buf, "lpush lkey");
        for (i = 0; i < 1000; i++) {
                strcat(buf, " ");
                strcat(buf, test_data[i]->ptr);
        }

        _case_answer_string_equal(handlerA, buf, "1000");
        _case_answer_string_equal(handlerB, buf, "1000");
        _case_answer_string_equal(handlerC, buf, "1000");


        snprintf(buf, sizeof(buf), "lrange lkey 0 -1");
        ans = kv_ask(handlerA, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 1000);
        kv_answer_iter_t *iter = kv_answer_get_iter(ans, ANSWER_TAIL);
        for (i = 0; i < kv_answer_length(ans); i++) {
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), test_data[i]->ptr);
        }
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        snprintf(buf, sizeof(buf), "lrange lkey 0 99");
        ans = kv_ask(handlerB, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 100);
        iter = kv_answer_get_iter(ans, ANSWER_HEAD);
        
        kv_answer_value_t *value;
        i = 999;
        while((value = kv_answer_next(iter)) != NULL) {
            CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), test_data[i--]->ptr);
            break;
        }
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        snprintf(buf, sizeof(buf), "lrange lkey 900 999");
        ans = kv_ask(handlerC, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 100);
        iter = kv_answer_get_iter(ans, ANSWER_TAIL);
        i = 0;
        while((value = kv_answer_next(iter)) != NULL) {
            CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(value), test_data[i++]->ptr);
            break;
        }
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        for (i = 0; i < 1000; i++) {
                case_data_release(test_data[i]);
        }

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lrange_stress()
{
        int i;
        case_data_t *test_data[2000];
        kv_handler_t *handlerA, *handlerB, *handlerC;
        char buf[2000*2001];
        kv_answer_t *ans;
        
        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        for (i = 0; i < 2000; i++) {
                test_data[i] = generator_rand_str(i+1);
        }

        strcpy(buf, "lpush lkey");
        for (i = 0; i < 2000; i++) {
                strcat(buf, " ");
                strcat(buf, test_data[i]->ptr);
        }
        
        _case_answer_string_equal(handlerA, buf, "2000");
        _case_answer_string_equal(handlerB, buf, "2000");
        _case_answer_string_equal(handlerC, buf, "2000");


        snprintf(buf, sizeof(buf), "lrange lkey 0 -1");
        ans = kv_ask(handlerA, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 2000);
        kv_answer_iter_t *iter = kv_answer_get_iter(ans, ANSWER_TAIL);
        for (i = 0; i < kv_answer_length(ans); i++) {
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), test_data[i]->ptr);
        }
        kv_answer_release_iter(iter);
        kv_answer_release(ans);
        
        snprintf(buf, sizeof(buf), "lrange lkey 0 -1");
        ans = kv_ask(handlerB, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 2000);
        iter = kv_answer_get_iter(ans, ANSWER_TAIL);
        for (i = 0; i < kv_answer_length(ans); i++) {
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), test_data[i]->ptr);
        }
        kv_answer_release_iter(iter);
        kv_answer_release(ans);

        snprintf(buf, sizeof(buf), "lrange lkey 0 -1");
        ans = kv_ask(handlerC, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_EQUAL(kv_answer_length(ans), 2000);
        iter = kv_answer_get_iter(ans, ANSWER_TAIL);
        for (i = 0; i < kv_answer_length(ans); i++) {
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), test_data[i]->ptr);
        }

        kv_answer_release_iter(iter);
        kv_answer_release(ans);
        
        for (i = 0; i < 2000; i++) {
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

        UTIL_ADD_CASE_SHORT(case_cmd_lrange_without_found);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_found);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_lrange_multi_db);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_lrange_stress);

        UTIL_RUN();
        UTIL_UNINIT();

        return 0;
}
#endif
