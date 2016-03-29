#include "ut_public.h"


void case_cmd_hgetall()
{
	kv_handler_t *handler;
	kv_answer_t *ans;
	kv_answer_iter_t *iter;

	handler = kv_create(NULL);
	/** hgetall->none*/
	ans = kv_ask(handler, "hgetall key", strlen("hgetall key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
	CU_ASSERT_EQUAL(kv_answer_length(ans), 0);
	CU_ASSERT_PTR_EQUAL(kv_answer_first_value(ans), NULL);
	kv_answer_release(ans);
	/** hset->hgetall*/
	_case_answer_string_equal(handler, "hset key field value", "1");
	ans = kv_ask(handler, "hgetall key", strlen("hgetall key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	iter = kv_answer_get_iter(ans, ANSWER_HEAD);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field");
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value");
	kv_answer_release_iter(iter);
	kv_answer_release(ans);
	/** hmset->hgetall*/
	_case_answer_string_equal(handler, "hmset key field value field1 value1 field2 value2", "OK");
	ans = kv_ask(handler, "hgetall key", strlen("hgetall key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	iter = kv_answer_get_iter(ans,ANSWER_HEAD);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field");
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value");
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field1");
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value1");
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field2");
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value2");
	kv_answer_release_iter(iter);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "hmset key field value0123456789012345678901234567890123456789012345678901234567890123456789", "OK");
	ans = kv_ask(handler, "hgetall key", strlen("hgetall key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	iter = kv_answer_get_iter(ans,ANSWER_HEAD);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field");
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value0123456789012345678901234567890123456789012345678901234567890123456789");
	kv_answer_release_iter(iter);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);		
}

void case_cmd_hgetall_invalid_cmd()
{
	kv_handler_t *handler;
	kv_answer_t *ans;
	
	handler = kv_create(NULL);

	ans = kv_ask(handler, "hgeta key", strlen("hgeta key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	kv_answer_release(ans);
	
	_case_answer_string_equal(handler, "hset key field value", "1");
	ans = kv_ask(handler, "hgetal key", strlen("hgetal key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hgetall_invalid_param()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	ans = kv_ask(handler, "hgetall key field ", strlen("hgetall key field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hgetall_multi_db()
{
	kv_handler_t *handler;
	kv_answer_iter_t *iter;
	kv_answer_t *ans;
	
	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "select 0", "OK");	
	_case_answer_string_equal(handler, "hset key field value", "1");
	ans = kv_ask(handler, "hgetall key", strlen("hgetall key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	iter = kv_answer_get_iter(ans, ANSWER_HEAD);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field");
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value");
	kv_answer_release_iter(iter);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "select 8", "OK");
	_case_answer_string_equal(handler, "hset key field value", "1");
	ans = kv_ask(handler, "hgetall key", strlen("hgetall key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	iter = kv_answer_get_iter(ans, ANSWER_HEAD);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field");
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value");
	kv_answer_release_iter(iter);
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);	
}

void case_cmd_hgetall_multi_handler()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_answer_iter_t *iter;
	kv_answer_t *ans;

	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);

	_case_answer_string_equal(handlerA, "select 0", "OK");
	_case_answer_string_equal(handlerA, "hset key field value", "1");
	ans = kv_ask(handlerA, "hgetall key", strlen("hgetall key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	iter = kv_answer_get_iter(ans, ANSWER_HEAD);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field");
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value");
	kv_answer_release_iter(iter);
	kv_answer_release(ans);

	_case_answer_string_equal(handlerB, "select 8", "OK");
	_case_answer_string_equal(handlerB, "hset key field value", "1");
	ans = kv_ask(handlerB, "hgetall key", strlen("hgetall key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	iter = kv_answer_get_iter(ans, ANSWER_HEAD);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field");
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value");
	kv_answer_release_iter(iter);
	kv_answer_release(ans);
	
	kv_destroy(handlerA);
	kv_destroy(handlerB);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);	
	
}

void case_cmd_hgetall_stress()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_handler_t *handlerC;

	kv_answer_t *ans;
	kv_answer_iter_t *iter;
	case_data_t *test_data[1024*2];
        static char cmd[1024*2048+1024];
	int i;

	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);
	handlerC = kv_create(NULL);

	for (i = 0; i < 2*1024; i++) {
                test_data[i] = generator_rand_str(i+1);
        }

        for (i = 0; i < 64-32; i++) {
                snprintf(cmd, sizeof(cmd), "hset hkey%d field value", i);
                _case_answer_string_equal(handlerA, cmd, "1");
                _case_answer_string_equal(handlerB, cmd, "1");
                _case_answer_string_equal(handlerC, cmd, "1");

                snprintf(cmd, sizeof(cmd), "hgetall hkey%d", i);
                ans = kv_ask(handlerA, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	        iter = kv_answer_get_iter(ans,ANSWER_HEAD);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field");
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value");
                kv_answer_release(ans);
		kv_answer_release_iter(iter);

                ans = kv_ask(handlerB, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	        iter = kv_answer_get_iter(ans,ANSWER_HEAD);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field");
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value");
                kv_answer_release(ans);
		kv_answer_release_iter(iter);
                                
                ans = kv_ask(handlerC, cmd, strlen(cmd));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	        iter = kv_answer_get_iter(ans,ANSWER_HEAD);
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "field");
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "value");
                kv_answer_release(ans);
		kv_answer_release_iter(iter);
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
	
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall);
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_hgetall_stress);

	UTIL_RUN();
	UTIL_UNINIT();

	return 0;
}
#endif
