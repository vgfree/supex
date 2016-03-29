#include "ut_public.h"


void case_cmd_hdel()
{
	kv_handler_t *handler;
	kv_answer_t *ans;
	size_t length;
	
	handler = kv_create(NULL);

	ans = kv_ask(handler, "hdel key not-exists-field", strlen("hdel key not-exists-field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
	CU_ASSERT_EQUAL(kv_answer_length(ans), 1);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "hmset key exists_field value", "OK");
	ans = kv_ask(handler, "hdel key exists_field", strlen("hdel key exists_field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "hmset key field value field2 value2 field3 value3", "OK");
	ans = kv_ask(handler, "hdel key field field2 field3", strlen("hdel key field field2 field3"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "3");
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hdel_invalid_param()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	ans = kv_ask(handler, "hdel key", strlen("hdel key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}


void case_cmd_hdel_invalid_cmd()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	ans = kv_ask(handler, "hde key field", strlen("hde key field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hdel_multi_db()
{
	
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "select 0", "OK");
	_case_answer_string_equal(handler, "hmset 0key field value", "OK");
	ans = kv_ask(handler, "hdel 0key field", strlen("hdel 0key field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "select 8", "OK");
	_case_answer_string_equal(handler, "hmset 8key field value", "OK");
	ans = kv_ask(handler, "hdel 8key field", strlen("hdel 8key field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hdel_multi_handler()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_answer_t *ans;

	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);

	_case_answer_string_equal(handlerA, "select 0", "OK");
	_case_answer_string_equal(handlerA, "hmset 0key field value", "OK");
	ans = kv_ask(handlerA, "hdel 0key field", strlen("hdel 0key field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
	kv_answer_release(ans);

	_case_answer_string_equal(handlerB, "select 8", "OK");
	_case_answer_string_equal(handlerB, "hmset 8key field value", "OK");
	ans = kv_ask(handlerB, "hdel 8key field", strlen("hdel 8key field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
	kv_answer_release(ans);

	kv_destroy(handlerA);
	kv_destroy(handlerB);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hdel_stress()
{
	int i;
	char cmdbuf[3000+128];
	char msgbuf[3000+128];
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_answer_t *ans;
	
	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);

	for (i=0; i<3000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "hmset key%d field%d value%d", i, i, i);
		snprintf(msgbuf, sizeof(msgbuf), "hdel key%d field%d", i, i, i);
		_case_answer_string_equal(handlerA, cmdbuf, "OK");
		_case_answer_string_equal(handlerA, msgbuf, "1");
	}
	
	_case_answer_string_equal(handlerB, "hmset key field value0123456789012345678901234567890123456789012345678901234567890123456789", "OK");
	ans = kv_ask(handlerB, "hdel key field", strlen("hdel key field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
	kv_answer_release(ans);

	kv_destroy(handlerA);
	kv_destroy(handlerB);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
	
}

#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
	UTIL_INIT_DEFAULT();

	UTIL_ADD_CASE_SHORT(case_cmd_hdel);
	UTIL_ADD_CASE_SHORT(case_cmd_hdel_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_hdel_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_hdel_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_hdel_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_hdel_stress);

	UTIL_RUN();
	UTIL_UNINIT();

	return 0;
}
#endif
