#include <unistd.h>
#include "ut_public.h"



void case_cmd_echo()
{
	kv_handler_t *handler;
	kv_answer_t *ans;
	
	
	handler = kv_create(NULL);
	
	ans = kv_ask(handler, "echo key",strlen("echo key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "key");
	kv_answer_release(ans);
	/*	
	ans = kv_ask(handler,"echo hello word",strlen("echo hello word"));
	CU_ASSERT_EQUAL(ans->errnum,ERR_NONE);
	printf("===>%d\n", kv_answer_length(ans));
	printf("===>%s\n", kv_answer_value_to_string(kv_answer_first_value(ans)));
	kv_answer_release(ans);
	*/
	_case_answer_string_equal(handler, "set key 1","OK");
	ans = kv_ask(handler, "echo key", strlen("echo key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "key");
	kv_answer_release(ans);
	_case_answer_string_equal(handler, "dbsize","1");

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(),0);
}

void case_cmd_echo_invalid_param()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler=kv_create(NULL);

	ans = kv_ask(handler,"echo", strlen("echo"));
	CU_ASSERT_EQUAL(ans->errnum,  ERR_ARGUMENTS);
	CU_ASSERT_PTR_EQUAL(ans->head, NULL);
	CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
	kv_answer_release(ans);
	
	ans = kv_ask(handler, "echo key", strlen("echo key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	ans = kv_ask(handler, "echo key value", strlen("echo key value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	CU_ASSERT_PTR_EQUAL(ans->head, NULL);
	CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_echo_invalid_cmd()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler=kv_create(NULL);

	ans = kv_ask(handler, "ech1 hello", strlen("ech1 hello"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	CU_ASSERT_PTR_EQUAL(ans->head, NULL);
	CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_echo_multi_db()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);
	
	_case_answer_string_equal(handler, "select 8", "OK");
	ans = kv_ask(handler, "echo key", strlen("echo key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "key");
	kv_answer_release(ans);
	_case_answer_string_equal(handler, "dbsize","0");
	
	_case_answer_string_equal(handler, "select 15", "OK");
	ans = kv_ask(handler, "echo key_", strlen("echo key_"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "key_");
	kv_answer_release(ans);
	_case_answer_string_equal(handler, "dbsize","0");

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_echo_multi_handler()
{
	int i;
	char cmdbuf[10000+128];
	char valbuf[10000+128];
	kv_handler_t *handlerA, *handlerB, *handlerC;

	kv_answer_t *ans;
	
	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);
	handlerC = kv_create(NULL);

	for(i=0; i<3000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "echo key%d", i);
		snprintf(valbuf, sizeof(valbuf), "key%d", i);
		ans = kv_ask(handlerA, cmdbuf, strlen(cmdbuf));
		CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), valbuf);
		kv_answer_release(ans);
	}
	
	for(i=0; i<2000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "echo key%d", i);
		snprintf(valbuf, sizeof(valbuf), "key%d", i);
		ans = kv_ask(handlerB, cmdbuf, strlen(cmdbuf));
		CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), valbuf);
		kv_answer_release(ans);
	}
	
	for(i=0; i<1000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "echo key%d", i);
		snprintf(valbuf, sizeof(valbuf), "key%d", i);
		ans = kv_ask(handlerC, cmdbuf, strlen(cmdbuf));
		CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), valbuf);
		kv_answer_release(ans);
	}
	
	kv_destroy(handlerA);
	kv_destroy(handlerB);
	kv_destroy(handlerC);

	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_echo_stress()
{	
		
	int i;
	char cmdbuf[1024];
	char valbuf[1024];
	kv_handler_t *handler;
	
	kv_answer_t *ans;
	
	handler = kv_create(NULL);
	
	for(i=0; i<1000; i++) {
		snprintf(cmdbuf, sizeof(cmdbuf), "echo key%d", i);
		snprintf(valbuf, sizeof(valbuf), "key%d", i);
		ans = kv_ask(handler, cmdbuf, strlen(cmdbuf));
		CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
		CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), valbuf);
		kv_answer_release(ans);
	}

	_case_answer_string_equal(handler, "select 15", "OK");
	ans = kv_ask(handler, "echo key0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", strlen("echo key0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "key0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
	kv_answer_release(ans);
	_case_answer_string_equal(handler, "dbsize","0");
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

#ifndef ENABLE_AUTO
int main(int argc,char *argv[])
{	
	UTIL_INIT_DEFAULT();
	UTIL_ADD_CASE_SHORT(case_cmd_echo);
	UTIL_ADD_CASE_SHORT(case_cmd_echo_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_echo_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_echo_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_echo_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_echo_stress);
	
	UTIL_RUN();
	UTIL_UNINIT();

	return 0;
}

#endif
