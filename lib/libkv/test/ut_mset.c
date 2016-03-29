#include "ut_public.h"
#include <unistd.h>

void case_cmd_mset()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "mset key value", "OK");
	_case_answer_string_equal(handler, "mset key1 value1 key2 value2 key3 value3 key4 value4", "OK");
	
	ans = kv_ask(handler, "mset key value key2 value2", strlen("mset key value key2 value2"));
	CU_ASSERT_STRING_EQUAL("OK", kv_answer_value_to_string(kv_answer_first_value(ans)));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	ans = kv_ask(handler, "mset key value key2 value2 key3 value3 key4 value4 key5 value5", strlen("mset key value key2 value2 key3 value3 key4 value4 key5 value5"));
	CU_ASSERT_STRING_EQUAL("OK", kv_answer_value_to_string(kv_answer_first_value(ans)));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	_case_answer_string_equal(handler, "get key", "value");
	_case_answer_string_equal(handler, "get key2", "value2");
	_case_answer_string_equal(handler, "get key3", "value3");
	_case_answer_string_equal(handler, "get key4", "value4");
	_case_answer_string_equal(handler, "get key5", "value5");

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_mset_invalid_param()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);
	
	ans = kv_ask(handler, "mset", strlen("mset"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	CU_ASSERT_PTR_EQUAL(ans->head, NULL);
	CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
	kv_answer_release(ans);

	ans = kv_ask(handler, "mset key", strlen("mset key"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	CU_ASSERT_PTR_EQUAL(ans->head, NULL);
	CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
	kv_answer_release(ans);
	
	ans = kv_ask(handler, "mset key value", strlen("mset key value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	ans = kv_ask(handler, "mset key1 value more", strlen("mset key1 value more"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	CU_ASSERT_PTR_EQUAL(ans->head, NULL);
	CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
	kv_answer_release(ans);

	ans = kv_ask(handler, "mset key1 value key2 value2 key3", strlen("mset key1 value key2 value2 key3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
	CU_ASSERT_PTR_EQUAL(ans->head, NULL);
	CU_ASSERT_PTR_EQUAL(ans->tail, NULL);
	kv_answer_release(ans);
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_mset_invalid_cmd()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);
	
	ans = kv_ask(handler, "mse key value", strlen("mse key value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(),0);
}

void case_cmd_mset_multi_handler()
{
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;

	kv_answer_t *ans;

	handlerA = kv_create(NULL);
	handlerB = kv_create(NULL);
	
	ans = kv_ask(handlerA, "mset key value", strlen("mset key value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);
	
	ans = kv_ask(handlerB, "mset key value", strlen("mset key value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	kv_destroy(handlerA);
	kv_destroy(handlerB);

	CU_ASSERT_EQUAL(kv_get_used_memory(),0);
}

void case_cmd_mset_multi_db()
{
	kv_handler_t *handler;
	kv_answer_t *ans;

	handler = kv_create(NULL);

	_case_answer_string_equal(handler, "select 0","OK");
	ans = kv_ask(handler, "mset key value", strlen("mset key value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	_case_answer_string_equal(handler, "expire key 1", "1");
	usleep((useconds_t)1.1*1E6);
	_case_answer_string_equal(handler, "dbsize", "0");
	kv_answer_release(ans);

	_case_answer_string_equal(handler, "select 5","OK");
	ans = kv_ask(handler, "mset key value", strlen("mset key value"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	kv_answer_release(ans);

	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);

}

void case_cmd_mset_stress()
{
	kv_answer_t *ans;
	kv_handler_t *handlerA;
	kv_handler_t *handlerB;
	kv_handler_t *handlerC;


	char buf[2000+128];	
	char getbuf[2000+128];	
	char valuebuf[2000+128];	
	int index;
	
	CU_ASSERT_PTR_NOT_EQUAL(handlerA= kv_create(NULL), NULL);
	CU_ASSERT_PTR_NOT_EQUAL(handlerB = kv_create(NULL), NULL);
	CU_ASSERT_PTR_NOT_EQUAL(handlerC = kv_create(NULL), NULL);

	for(index = 0; index <=1000; index++) {
		snprintf(buf, sizeof(buf), "mset key%d %d mkey%d m%d", index,index,index,index);
		_case_answer_string_equal(handlerA, buf, "OK");
		snprintf(getbuf,sizeof(getbuf), "get key%d", index);
		snprintf(valuebuf, sizeof(valuebuf), "%d", index);
		ans = kv_ask(handlerA, getbuf, strlen(getbuf));
		CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
		CU_ASSERT_STRING_EQUAL(valuebuf, kv_answer_value_to_string(kv_answer_first_value(ans)));
		kv_answer_release(ans);
	}
	
	for(index = 0; index <=2000; index++) {
		snprintf(buf, sizeof(buf), "mset key%d value%d mkey%d mvalue%d", index,index,index,index);
		_case_answer_string_equal(handlerB, buf, "OK");
		snprintf(getbuf,sizeof(getbuf), "get mkey%d", index);
		snprintf(valuebuf, sizeof(valuebuf), "mvalue%d", index);
		ans = kv_ask(handlerB, getbuf, strlen(getbuf));
		CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
		CU_ASSERT_STRING_EQUAL(valuebuf, kv_answer_value_to_string(kv_answer_first_value(ans)));
		kv_answer_release(ans);
	}
	
	_case_answer_string_equal(handlerC, "mset keyabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz0123456789 value", "OK");
		
	kv_destroy(handlerA);
	kv_destroy(handlerB);
	kv_destroy(handlerC);
	CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

#ifndef ENABLE_AUTO
int main(int argc, char *agrv[])
{
	UTIL_INIT_DEFAULT();

	UTIL_ADD_CASE_SHORT(case_cmd_mset);
	UTIL_ADD_CASE_SHORT(case_cmd_mset_invalid_param);
	UTIL_ADD_CASE_SHORT(case_cmd_mset_invalid_cmd);
	UTIL_ADD_CASE_SHORT(case_cmd_mset_multi_handler);
	UTIL_ADD_CASE_SHORT(case_cmd_mset_multi_db);
	UTIL_ADD_CASE_SHORT(case_cmd_mset_stress);

	UTIL_RUN();
	UTIL_UNINIT();

	return 0;	
}
#endif
