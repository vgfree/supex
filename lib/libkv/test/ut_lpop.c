#include "ut_public.h"
#include <unistd.h>

void case_cmd_lpop()
{
	kv_handler_t *handler;
	kv_answer_iter_t *iter;
	kv_answer_t *ans;

	handler = kv_create(NULL);
	
	_case_answer_string_equal(handler, "rpush key c01234567890123456789abcdefghijklmnopqrstuvwxyz012345678901234567890123456789 a b", "3");
	ans = kv_ask(handler, "lpop key", strlen("lpop key"));
	CU_ASSERT_EQUAL(ans->errnum,ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "c01234567890123456789abcdefghijklmnopqrstuvwxyz012345678901234567890123456789");
	kv_answer_release(ans);

	ans = kv_ask(handler, "lpop rkey", strlen("lpop rkey"));
	CU_ASSERT_EQUAL(ans->errnum,ERR_NIL);
	kv_answer_release(ans);
	
	_case_answer_string_equal(handler, "rpush rkey 1 2 3", "3");
	ans = kv_ask(handler, "lpop rkey", strlen("lpop rkey"));
	CU_ASSERT_EQUAL(ans->errnum,ERR_NONE);
	iter = kv_answer_get_iter(ans, ANSWER_HEAD);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_next(iter)), "1");
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "lpush lkey a b c", "3");
	ans = kv_ask(handler, "lpop lkey", strlen("lpop lkey"));
	CU_ASSERT_EQUAL(ans->errnum,ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "c");
	kv_answer_release(ans);
	
	kv_answer_release_iter(iter);
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lpop_invalid_param()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);
	
	_case_answer_string_equal(handler, "lpush key a b c", "3");
	
	ans = kv_ask(handler, "lpop key a", strlen("lpop key a"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lpop_invalid_type()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "set key value", "OK");

	ans = kv_ask(handler, "lpop key", strlen("lpop key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lpop_invalid_cmd()
{	
	kv_handler_t *handler;
	kv_answer_t *ans;
	
	handler = kv_create(NULL);
	
	_case_answer_string_equal(handler, "lpush key a b c", "3");

	ans = kv_ask(handler, "lpo key", strlen("lpo key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lpop_nil()
{
	kv_handler_t *handler;
	kv_answer_t *ans;
	
	handler = kv_create(NULL);
	
	ans = kv_ask(handler, "lpop key", strlen("lpop key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lpop_expire()
{
	kv_handler_t *handler;
	kv_answer_t *ans;
	
	handler = kv_create(NULL);
	
	_case_answer_string_equal(handler, "lpush llkey a b c", "3");
	_case_answer_string_equal(handler, "expire llkey 1", "1");

	usleep(1.1*1E6);
	ans = kv_ask(handler, "lpop llkey", strlen("lpop llkey"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lpop_multi_db()
{
	kv_handler_t *handler;
	kv_answer_t *ans;
	
	handler = kv_create(NULL);
	
	_case_answer_string_equal(handler, "select 2", "OK");
	_case_answer_string_equal(handler, "lpush key a b c", "3");

	ans = kv_ask(handler, "lpop key", strlen("lpop key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "c");
	kv_answer_release(ans);
	
	_case_answer_string_equal(handler, "select 8", "OK");
	_case_answer_string_equal(handler, "lpush key a b c", "3");

	ans = kv_ask(handler, "lpop key", strlen("lpop key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "c");
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lpop_multi_handler()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_answer_t *ans;
	
	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);
	
	_case_answer_string_equal(handlerA, "lpush key a b c", "3");

	ans = kv_ask(handlerA, "lpop key", strlen("lpop key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	_case_answer_string_equal(handlerB, "lpush key a b c", "3");

	ans = kv_ask(handlerB, "lpop key", strlen("lpop key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	kv_destroy(handlerA);
	kv_destroy(handlerB);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_lpop_stress()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_handler_t *handlerC;
	kv_answer_t *ans;
	int i;
	char cmdbuf[1000+128];
	char msgbuf[1000+128];
	char buf[1000+128];

	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);
	handlerC = kv_create(NULL);
	
	for(i=0; i<1000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "lpush lkey%d a b c", i);
		snprintf(msgbuf, sizeof(msgbuf), "lpop lkey%d", i);
		_case_answer_string_equal(handlerA, cmdbuf, "3");
		_case_answer_string_equal(handlerA, msgbuf, "c");
	}
	
	for(i=0; i<2000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "lpush lkey%d a%d b%d c", i, i, i);
		snprintf(msgbuf, sizeof(msgbuf), "lpop lkey%d", i);
		_case_answer_string_equal(handlerB, cmdbuf, "3");
		_case_answer_string_equal(handlerB, msgbuf, "c");
	}
	
	for(i=0; i<3000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "lpush lkey%d a%d b%d c%d", i, i, i,i);
		snprintf(msgbuf, sizeof(msgbuf), "lpop lkey%d", i);
		snprintf(buf, sizeof(buf),"c%d", i);
		_case_answer_string_equal(handlerC, cmdbuf, "3");
		_case_answer_string_equal(handlerC, msgbuf, buf);
	}
	
	_case_answer_string_equal(handlerA, "dbsize", "1000");
	_case_answer_string_equal(handlerB, "dbsize", "2000");
	_case_answer_string_equal(handlerC, "dbsize", "3000");

	kv_destroy(handlerA);
	kv_destroy(handlerB);
	kv_destroy(handlerC);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

#ifndef ENABLE_AUTO
int main(int argc,char *argv[])
{
	UTIL_INIT_DEFAULT();

	UTIL_ADD_CASE_SHORT(case_cmd_lpop);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_invalid_type);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_nil);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_expire);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_lpop_stress);
	
	UTIL_RUN();
	UTIL_UNINIT();
}
#endif
