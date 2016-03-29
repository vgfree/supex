#include "ut_public.h"

void cmd_case_srem()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	ans = kv_ask(handler, "srem key non_exits_member", strlen("srem key non_exits_member"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
	kv_answer_release(ans);
	
	_case_answer_string_equal(handler, "sadd key a b c", "3");
	ans = kv_ask(handler, "srem key a", strlen("srem key a"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
	kv_answer_release(ans);
		
	ans = kv_ask(handler, "srem key b", strlen("srem key b"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	ans = kv_ask(handler, "srem key c", strlen("srem key c"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "sadd skey aa bbb ccc dddd", "4");
	ans = kv_ask(handler, "srem skey aa bbb ccc dddd", strlen("srem skey aa bbb ccc dddd"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "4");
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "sadd skey aa bbb ccc dddd0123456789012345678901234567890123456789012345678901234567890123456789", "4");
	ans = kv_ask(handler, "srem skey aa bbb ccc dddd0123456789012345678901234567890123456789012345678901234567890123456789", strlen("srem skey aa bbb ccc dddd0123456789012345678901234567890123456789012345678901234567890123456789"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "4");
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void cmd_case_srem_invalid_cmd()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	ans = kv_ask(handler, "sre key non_exits_member", strlen("sre key non_exits_member"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	kv_answer_release(ans);
	
	_case_answer_string_equal(handler, "sadd key a b c", "3");
	ans = kv_ask(handler, "sre key a", strlen("sre key a"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);

	kv_answer_release(ans);
		
	ans = kv_ask(handler, "srm key b", strlen("srm key b"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	kv_answer_release(ans);

	ans = kv_ask(handler, "sremm key c", strlen("sremm key c"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "sadd skey aa bbb ccc dddd", "4");
	ans = kv_ask(handler, "sremee skey aa bbb ccc dddd", strlen("sremee skey aa bbb ccc dddd"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
} 
void cmd_case_srem_invalid_param()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	ans = kv_ask(handler, "srem", strlen("srem"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);
	
	_case_answer_string_equal(handler, "sadd key a b c", "3");
	ans = kv_ask(handler, "srem key", strlen("srem key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);
		
	ans = kv_ask(handler, "srem", strlen("srem"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);

	ans = kv_ask(handler, "srem key", strlen("srem key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "sadd skey aa bbb ccc dddd", "4");
	ans = kv_ask(handler, "srem skey", strlen("srem skey"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void cmd_case_srem_invalid_type()
{	
	kv_handler_t *handler;
	kv_answer_t *ans;
	
	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "lpush key a b c", "3");
	ans = kv_ask(handler, "srem key a b c", strlen("srem key a b c"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void cmd_case_srem_multi_db()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "select 0", "OK");
	_case_answer_string_equal(handler, "sadd key a b c d", "4");
	ans = kv_ask(handler, "srem key a b c d", strlen("srem key a b c d"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	_case_answer_string_equal(handler, "select 15", "OK");
	_case_answer_string_equal(handler, "sadd key a b c d", "4");
	ans = kv_ask(handler, "srem key a b c d", strlen("srem key a b c d"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);	
}
void cmd_case_srem_multi_handler()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_answer_t *ans;

	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);

	_case_answer_string_equal(handlerA, "select 0", "OK");
	_case_answer_string_equal(handlerA, "sadd key a b c d", "4");
	ans = kv_ask(handlerA, "srem key a b c d", strlen("srem key a b c d"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	_case_answer_string_equal(handlerB, "select 15", "OK");
	_case_answer_string_equal(handlerB, "sadd key a b c d", "4");
	ans = kv_ask(handlerB, "srem key a b c d", strlen("srem key a b c d"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	kv_destroy(handlerA);
	kv_destroy(handlerB);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);	
}

void cmd_case_srem_stress()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_answer_t *ans;
	int i;
	case_data_t *test_data[1024*2];
	static char cmd[2048+1024];
	static char cmd1[2048+1024];
	
	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);

	for (i = 0; i< 2*1024; i++) {
		test_data[i] = generator_rand_str(i+1);
	}

	for (i = 0; i < 10000; i++) {
		snprintf(cmd, sizeof(cmd), "sadd skey v%d", i+1);
		_case_answer_string_equal(handlerA, cmd, "1");
		_case_answer_string_equal(handlerB, cmd, "1");
		
		snprintf(cmd1, sizeof(cmd1), "srem skey v%d", i+1);
		ans = kv_ask(handlerA, cmd1, strlen(cmd1));
		CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
		kv_answer_release(ans);
	}

	for (i = 0; i < 10; i++) {
		snprintf(cmd, sizeof(cmd), "sadd skey v%d", test_data[i]->ptr);
		_case_answer_string_equal(handlerA, cmd, "1");
		_case_answer_string_equal(handlerB, cmd, "1");
		
		snprintf(cmd, sizeof(cmd), "srem skey v%d", test_data[i]->ptr);
		ans = kv_ask(handlerA, cmd, strlen(cmd));
		CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
		kv_answer_release(ans);
		ans = kv_ask(handlerB, cmd, strlen(cmd));
		CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
		kv_answer_release(ans);
	}
	
	for (i = 0; i < 2*1024; i++) {
                case_data_release(test_data[i]);
        }

	kv_destroy(handlerA);
	kv_destroy(handlerB);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}
#ifndef ENABLE_AUTO
int main()
{
	UTIL_INIT_DEFAULT();

	UTIL_ADD_CASE_SHORT(cmd_case_srem);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_invalid_cmd);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_invalid_param);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_invalid_type);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_multi_db);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_multi_handler);
	UTIL_ADD_CASE_SHORT(cmd_case_srem_stress);

	UTIL_RUN();
	UTIL_UNINIT();

	return 0;
}
#endif
