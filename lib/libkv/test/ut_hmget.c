#include "ut_public.h"

void case_cmd_hmget()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);
	
	ans = kv_ask(handler, "hmget newkey null_field", strlen("hmget newkey null_field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
	kv_answer_release(ans);	

	_case_answer_string_equal(handler, "hmset create_key field111 value", "OK");
	ans = kv_ask(handler, "hmget create_key field111", strlen("hmget create_key field111"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "value");
	kv_answer_release(ans);	

	_case_answer_string_equal(handler, "hmset create_key field111 value01234567890123456789012345678901234567890123456789012345678901234567890123456789", "OK");
	ans = kv_ask(handler, "hmget create_key field111", strlen("hmget create_key field111"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "value01234567890123456789012345678901234567890123456789012345678901234567890123456789");
	kv_answer_release(ans);	
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hmget_invalid_param()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);
	
	ans = kv_ask(handler, "hmget key", strlen("hmget key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);	

	ans = kv_ask(handler, "hmget hmkey", strlen("hmget hmkey"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);	
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hmget_invalid_cmd()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	ans = kv_ask(handler, "hmge key field", strlen("hmge key field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hmget_multi_handler()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_answer_t *ans;

	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);
	
	_case_answer_string_equal(handlerA, "hmset Akey Afield Avalue", "OK");
	ans = kv_ask(handlerA, "hmget Akey Afield", strlen("hmget Akey Afield"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	_case_answer_string_equal(handlerB, "hmset Bkey Bfield Bvalue", "OK");
	ans = kv_ask(handlerB, "hmget Bkey Bfield", strlen("hmget Bkey Bfield"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	kv_destroy(handlerA);
	kv_destroy(handlerB);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hmget_multi_db()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);
	
	_case_answer_string_equal(handler,"select 0","OK");
	ans = kv_ask(handler, "hmget key field",strlen("hmget key field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "select 15", "OK");
	_case_answer_string_equal(handler, "hmset key field value", "OK");
	ans = kv_ask(handler, "hmget key field", strlen("hmget key field"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_hmget_stress()
{
	int i ;
	char cmdbuf[3000+128];
	char msgbuf[3000+128];
	char msgbufB[3000+128];
	char rbuf[1000+128];
	char r1buf[1000+128];
	char r2buf[1000+128];
	char r3buf[1000+128];
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_answer_iter_t *iter;
	kv_answer_t *ans;

	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);


	for(i = 1; i<3000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "hmset key%d field%d %d", i, i, i);
		snprintf(msgbuf, sizeof(msgbuf), "hmget key%d field%d", i, i);
		snprintf(msgbufB, sizeof(msgbufB), "%d", i);
		_case_answer_string_equal(handlerA, cmdbuf, "OK");
		_case_answer_string_equal(handlerA, msgbuf, msgbufB);
	}

	for(i=0; i<1000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "hmset key%d field%d value%d 1field%d 1value%d 2field%d 2value%d 3field%d 3value%d", i, i, i, i, i, i, i, i, i);
		_case_answer_string_equal(handlerB, cmdbuf, "OK");
		snprintf(msgbuf, sizeof(msgbuf), "hmget key%d field%d 1field%d 2field%d 3field%d", i, i, i, i, i);
		snprintf(rbuf, sizeof(rbuf), "value%d", i);
		snprintf(r1buf, sizeof(r1buf), "1value%d", i);
		snprintf(r2buf, sizeof(r2buf), "2value%d", i);
		snprintf(r3buf, sizeof(r3buf), "3value%d", i);
		ans = kv_ask(handlerB, msgbuf, strlen(msgbuf));
		iter = kv_answer_get_iter(ans, ANSWER_HEAD);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), rbuf);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), r1buf);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), r2buf);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), r3buf);
		kv_answer_release(ans);
		kv_answer_release_iter(iter);	
	}

	kv_destroy(handlerA);
	kv_destroy(handlerB);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);		
}

#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
	UTIL_INIT_DEFAULT();

	UTIL_ADD_CASE_SHORT(case_cmd_hmget);
	UTIL_ADD_CASE_SHORT(case_cmd_hmget_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_hmget_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_hmget_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_hmget_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_hmget_stress);

	UTIL_RUN();
	UTIL_UNINIT();
}
#endif
