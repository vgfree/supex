#include <unistd.h>
#include "ut_public.h"



void case_cmd_expire()
{
        int i;
        char buf[1024];
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        for (i = 1; i <= 10000; i++) {
                snprintf(buf, sizeof(buf), "set %d value", i);
                _case_answer_string_equal(handler, buf, "OK");

                snprintf(buf, sizeof(buf), "expire %d 10", i);
                _case_answer_string_equal(handler, buf, "1");
                
                snprintf(buf, sizeof(buf), "get %d", i);
                _case_answer_string_equal(handler, buf, "value");
        }

        _case_answer_string_equal(handler, "dbsize", "10000");
        _case_answer_string_equal(handler, "flushdb", "OK");
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_expire_get()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "set key value", "OK");

        ans = kv_ask(handler, "expire key 5", strlen("expire key 5"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        sleep(3);
        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "value");
        kv_answer_release(ans);

        sleep(3);

        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);

        _case_answer_string_equal(handler, "dbsize", "0");
        
        _case_answer_string_equal(handler, "set key value", "OK");
        
        ans = kv_ask(handler, "expire key 2", strlen("expire key 2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);
        sleep(3);

        ans = kv_ask(handler, "del key", strlen("del key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);

        kv_answer_release(kv_ask(handler, "set key value", strlen("set key value")));

        _case_answer_string_equal(handler, "del key", "1");

        _case_answer_string_equal(handler, "flushdb", "OK");

        ans = kv_ask(handler, "expire key 5", strlen("expire key 5"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_expire_list()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "lpush lkey a b c", "3");

        ans = kv_ask(handler, "expire lkey 1", strlen("expire lkey 1"));
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handler, "lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        sleep(2);
        ans = kv_ask(handler, "lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void case_cmd_expire_set()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        _case_answer_string_equal(handler, "sadd skey a b c", "3");

        ans = kv_ask(handler, "expire skey 1", strlen("expire skey 1"));
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handler, "smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        sleep(2);
        ans = kv_ask(handler, "smembers skey", strlen("smembers skey"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        kv_destroy(handler);
}

void case_cmd_expire_without_key()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        ans = kv_ask(handler, "expire key 10", strlen("expire key 10"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "0");
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_expire_invalid_cmd()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);
        ans = kv_ask(handler, "expir key 10", strlen("expir key 10"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_CMD);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_expire_invalid_param()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        
        handler = kv_create(NULL);

        ans = kv_ask(handler, "expire", strlen("expire"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans); 
        
        ans = kv_ask(handler, "expire key", strlen("expire key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);

        ans = kv_ask(handler, "expire key 10 20", strlen("expire key 10 20"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_ARGUMENTS);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);

        ans = kv_ask(handler, "expire key 123456789012345678901234567890", strlen("expire key 123456789012345678901234567890"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_VALUE);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler);
}

void case_cmd_expire_multi_handler()
{
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        _case_answer_string_equal(handlerA, "set keyA valueA", "OK");
        _case_answer_string_equal(handlerB, "set keyB valueB", "OK");
        _case_answer_string_equal(handlerC, "set keyC valueC", "OK");

        _case_answer_string_equal(handlerA, "get keyA", "valueA");
        _case_answer_string_equal(handlerB, "get keyB", "valueB");
        _case_answer_string_equal(handlerC, "get keyC", "valueC");
        
        ans = kv_ask(handlerA, "expire keyA 1", strlen("expire keyA 1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "expire keyB 3", strlen("expire keyB 3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        ans = kv_ask(handlerC, "expire keyC 5", strlen("expire keyC 5"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "get keyA", "valueA");
        _case_answer_string_equal(handlerB, "get keyB", "valueB");
        _case_answer_string_equal(handlerC, "get keyC", "valueC");

        usleep((useconds_t)1.1E6);
        ans = kv_ask(handlerA, "get keyA", strlen("get keyA"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        _case_answer_string_equal(handlerB, "get keyB", "valueB");
        _case_answer_string_equal(handlerC, "get keyC", "valueC");

        sleep(2);
        ans = kv_ask(handlerB, "get keyB", strlen("get keyB"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);
        
        _case_answer_string_equal(handlerC, "get keyC", "valueC");

        sleep(2);
        ans = kv_ask(handlerC, "get keyC", strlen("get keyC"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);
}

void case_cmd_expire_negative_param()
{
	kv_answer_t *ans;
	kv_handler_t *handler;

	handler = kv_create(NULL);

	_case_answer_string_equal(handler,"set key value","OK");

	ans = kv_ask(handler, "expire key 1", strlen("expire key 1"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
	kv_answer_release(ans);
	
	usleep((useconds_t)1.1E6);	
	_case_answer_string_equal(handler,"dbsize","0");	
	
	_case_answer_string_equal(handler,"set key1 value","OK");
	
	ans = kv_ask(handler, "expire key1 10", strlen("expire key1 10"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
	kv_answer_release(ans);

	ans = kv_ask(handler, "expire key1 -1", strlen("expire key1 -1"));
	CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
	kv_answer_release(ans);
	
	_case_answer_string_equal(handler,"dbsize","0");	
	
	_case_answer_string_equal(handler,"lpush lkey a b c","3");
	
	ans=kv_ask(handler, "expire lkey -1",strlen("expire lkey -1"));
	CU_ASSERT_EQUAL(ans->errnum,ERR_NONE);
	CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
	kv_answer_release(ans);

	_case_answer_string_equal(handler,"dbsize","0");
	
	kv_destroy(handler);
	CU_ASSERT_EQUAL(kv_get_used_memory(),0);
}

void case_cmd_expire_stress()
{
        int i;
        kv_handler_t *handler;
        kv_answer_t *ans;
        static char buf[10000+32];

        handler = kv_create(NULL);
        _case_answer_string_equal(handler, "set key value", "OK");


        for (i = 10000; i >= 1; i--) {
                snprintf(buf, sizeof(buf), "expire key %d", i);
                ans = kv_ask(handler, buf, strlen(buf));
                CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "1");
                kv_answer_release(ans);
        }

        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "value");
        kv_answer_release(ans);

        usleep((useconds_t)1.1*1E6);
        ans = kv_ask(handler, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        CU_ASSERT_PTR_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), NULL);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        
        UTIL_ADD_CASE_SHORT(case_cmd_expire);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_get);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_list);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_set);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_without_key);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_invalid_cmd);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_invalid_param);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_multi_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_expire_negative_param);
	UTIL_ADD_CASE_SHORT(case_cmd_expire_stress);
	
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
