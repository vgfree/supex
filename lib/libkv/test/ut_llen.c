#include <unistd.h>
#include "ut_public.h"

void case_cmd_llen()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "lpush lkey a b c", "3");

	ans = kv_ask(handler, "llen lkey", strlen("llen lkey"));
	CU_ASSERT_STRING_EQUAL("3", kv_answer_value_to_string(kv_answer_first_value(ans)));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "rpush lkey a b cc", "6");

	ans = kv_ask(handler, "llen lkey", strlen("llen lkey"));
	CU_ASSERT_STRING_EQUAL("6", kv_answer_value_to_string(kv_answer_first_value(ans)));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "rpush lkey1abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwsyzabcdefghijklmnopqrstuvwxyz a b 123456789abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz111", "3");
	ans = kv_ask(handler, "llen lkey1abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwsyzabcdefghijklmnopqrstuvwxyz", strlen("llen lkey1abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwsyzabcdefghijklmnopqrstuvwxyz"));
	CU_ASSERT_STRING_EQUAL("3", kv_answer_value_to_string(kv_answer_first_value(ans)));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_llen_multi_db()
{
	kv_handler_t *handler;
	kv_answer_t *ans;
	
	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "select 0", "OK");
	_case_answer_string_equal(handler, "lpush akey a", "1");
	_case_answer_string_equal(handler, "dbsize", "1");
	
	ans = kv_ask(handler, "llen akey", strlen("llen akey"));
	CU_ASSERT_STRING_EQUAL("1", kv_answer_value_to_string(kv_answer_first_value(ans)));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "select 4", "OK");
	_case_answer_string_equal(handler, "lpush bkey a b c", "3");
	
	ans = kv_ask(handler, "llen bkey", strlen("llen bkey"));
	CU_ASSERT_STRING_EQUAL("3", kv_answer_value_to_string(kv_answer_first_value(ans)));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "select 10", "OK");
	_case_answer_string_equal(handler, "lpush ckey a bb ccc dddd", "4");
	
	ans = kv_ask(handler, "llen ckey", strlen("llen ckey"));
	CU_ASSERT_STRING_EQUAL("4", kv_answer_value_to_string(kv_answer_first_value(ans)));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}
void case_cmd_llen_invalid_param()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);
	
	_case_answer_string_equal(handler, "lpush lkey a b c", "3");

	ans = kv_ask(handler, "llen", strlen("llen"));
	CU_ASSERT_EQUAL(ans->errnum,ERR_ARGUMENTS);
	CU_ASSERT_PTR_EQUAL(ans->head, NULL);
	CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
	kv_answer_release(ans);

	ans = kv_ask(handler, "llen lkey", strlen("llen lkey"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	ans = kv_ask(handler, "llen key value", strlen("llen key value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	CU_ASSERT_PTR_EQUAL(ans->head, NULL);
	CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(),0);
}

void case_cmd_llen_invalid_type()
{
		
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);
	
	_case_answer_string_equal(handler, "set key value" ,"OK");

	ans = kv_ask(handler, "llen key", strlen("llen key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_TYPE);
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(),0);
}

void case_cmd_llen_invalid_cmd()
{
	
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "lpush lkey a b c", "3");
	_case_answer_string_equal(handler, "expire lkey 1", "1");
	
	usleep(1.1*1E6);
	ans = kv_ask(handler, "lle key", strlen("lle key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	CU_ASSERT_EQUAL(ans->head, NULL);
	CU_ASSERT_EQUAL(ans->tail, NULL);
	kv_answer_release(ans);

	kv_destroy(handler);

	CU_ASSERT_EQUAL(kv_get_used_memory(),0);
}

void case_cmd_llen_nil()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);
	
	ans = kv_ask(handler, "llen key", strlen("llen key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(),0);
}

void case_cmd_llen_expire()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "lpush lkey a b c", "3");
	_case_answer_string_equal(handler, "expire lkey 1", "1");
	
	usleep(1.1*1E6);
	ans = kv_ask(handler, "llen lkey", strlen("llen lkey"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
	kv_answer_release(ans);

	kv_destroy(handler);
		
	CU_ASSERT_EQUAL(kv_get_used_memory(),0);
}

void case_cmd_llen_multi_handler()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;

	kv_answer_t *ans;
	
	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);

	_case_answer_string_equal(handlerA, "lpush keyA a b c", "3");
	_case_answer_string_equal(handlerB, "lpush keyB a b c", "3");
	
	ans = kv_ask(handlerA, "llen keyA", strlen("llen keyA"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL("3", kv_answer_value_to_string(kv_answer_first_value(ans)));
	kv_answer_release(ans);

	ans = kv_ask(handlerB, "llen keyB", strlen("llen keyB"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL("3", kv_answer_value_to_string(kv_answer_first_value(ans)));
	kv_answer_release(ans);

	kv_destroy(handlerA);
	kv_destroy(handlerB);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
	
}

void case_cmd_llen_stress()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_handler_t *handlerC;
	
	int i;
	char cmdbuf[3000+128];
	char msgbuf[3000+128];
	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);
	handlerC = kv_create(NULL);
	
	for(i=0; i<1000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "lpush lkey%d a b c", i);
		snprintf(msgbuf, sizeof(msgbuf), "llen lkey%d", i);
		_case_answer_string_equal(handlerA, cmdbuf, "3");
		_case_answer_string_equal(handlerA, msgbuf, "3");
	}
	
	for(i=0; i<2000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "lpush lkey%d a b c", i);
		snprintf(msgbuf, sizeof(msgbuf), "llen lkey%d", i);
		_case_answer_string_equal(handlerB, cmdbuf, "3");
		_case_answer_string_equal(handlerB, msgbuf, "3");
	}
	
	for(i=0; i<3000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "lpush lkey%d a b c", i);
		snprintf(msgbuf, sizeof(msgbuf), "llen lkey%d", i);
		_case_answer_string_equal(handlerC, cmdbuf, "3");
		_case_answer_string_equal(handlerC, msgbuf, "3");
	}
	
	_case_answer_string_equal(handlerA, "dbsize" ,"1000");
	_case_answer_string_equal(handlerB, "dbsize" ,"2000");
	_case_answer_string_equal(handlerC, "dbsize" ,"3000");

	kv_destroy(handlerA);	
	kv_destroy(handlerB);	
	kv_destroy(handlerC);	
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

#ifndef ENABLE_AUTO
int main(int argc,char *argv[])
{
	UTIL_INIT_DEFAULT();

	UTIL_ADD_CASE_SHORT(case_cmd_llen);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_invalid_type);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_nil);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_expire);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_llen_stress);
	
	UTIL_RUN();
	UTIL_UNINIT();	

	return 0;
}
#endif
