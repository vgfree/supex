#include "ut_public.h"

void case_cmd_hmset()
{
	kv_handler_t *handler;
	kv_answer_t *ans;
	
	handler = kv_create(NULL);
	
	_case_answer_string_equal(handler, "hmset key field value", "OK");
	ans = kv_ask(handler, "hmset key1 field value", strlen("hmset key1 field value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
	_case_answer_string_equal(handler, "hmget key field", "value");
	kv_answer_release(ans);

		
	_case_answer_string_equal(handler, "hmset key1 field1 value1 field2 value2", "OK");
	ans = kv_ask(handler, "hmset key1 field1 value1 field2 value2", strlen("hmset key1 field1 value1 field2 value2"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "hmset key1 field1 value1 field2 value2 field3 value3", "OK");
	ans = kv_ask(handler, "hmset key1 field1 value1 field2 value2 field3 value3", strlen("hmset key1 field1 value1 field2 value2 field3 value3"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hmset_invalid_cmd()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	ans = kv_ask(handler, "hmse key1 field value", strlen("hmse key1 field value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hmset_invalid_param()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	ans = kv_ask(handler, "hmset key field", strlen("hmset key field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);

	ans = kv_ask(handler, "hmset key1 field field2 field3", strlen("hmset key1 field field2 field3"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hmset_multi_handler()
{
	
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_answer_t *ans;

	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);

	ans = kv_ask(handlerA, "hmset key field value", strlen("hmset key field value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	ans = kv_ask(handlerB, "hmset key1 field value", strlen("hmset key1 field value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	kv_destroy(handlerA);
	kv_destroy(handlerB);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hmset_multi_db()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "select 0", "OK");
	ans = kv_ask(handler, "hmset key field value", strlen("hmset key field value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "select 15", "OK");
	ans = kv_ask(handler, "hmset key1 field value", strlen("hmset key1 field value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hmset_stress()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_answer_t *ans;
	char hbuf[2000+128];
	char hgetbuf[2000+128];
	char sbuf[2000+128];
	int index;
	
	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);

	for(index = 0; index <=1000; index++) {
		snprintf(hbuf, sizeof(hbuf), "hmset key%d field%d 0123abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz", index,index);
		_case_answer_string_equal(handlerA, hbuf, "OK");
		snprintf(hgetbuf, sizeof(hgetbuf), "hget key%d field%d", index, index);
		_case_answer_string_equal(handlerA, hgetbuf, "0123abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz");
	}

	for(index = 0; index <= 2000; index++) {
		snprintf(hbuf, sizeof(hbuf), "hmset key%d field%d value%d", index, index, index);
		_case_answer_string_equal(handlerB, hbuf, "OK");
		snprintf(hgetbuf, sizeof(hgetbuf), "hget key%d field%d", index, index);
		snprintf(sbuf, sizeof(sbuf), "value%d", index);
		_case_answer_string_equal(handlerB, hgetbuf, sbuf);
	}
	
	kv_destroy(handlerA);
	kv_destroy(handlerB);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
	UTIL_INIT_DEFAULT();

	UTIL_ADD_CASE_SHORT(case_cmd_hmset);
	UTIL_ADD_CASE_SHORT(case_cmd_hmset_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_hmset_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_hmset_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_hmset_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_hmset_stress);
	
	UTIL_RUN();
	UTIL_UNINIT();
}
#endif
