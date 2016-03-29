#include "ut_public.h"



void case_cmd_select()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "select 0", strlen("select 0"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        ans = kv_ask(handler, "select 1", strlen("select 1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "select 2", strlen("select 2"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "select 3", strlen("select 3"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "select 4", strlen("select 4"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "select 5", strlen("select 5"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "select 6", strlen("select 6"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "select 7", strlen("select 7"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "select 8", strlen("select 8"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);
        
        ans = kv_ask(handler, "select 9", strlen("select 9"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        ans = kv_ask(handler, "select 10", strlen("select 10"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        ans = kv_ask(handler, "select 11", strlen("select 11"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        ans = kv_ask(handler, "select 12", strlen("select 12"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        ans = kv_ask(handler, "select 13", strlen("select 13"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        ans = kv_ask(handler, "select 14", strlen("select 14"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        ans = kv_ask(handler, "select 15", strlen("select 15"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_select_invalid_index()
{
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        ans = kv_ask(handler, "select -1", strlen("select -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_DB_INDEX);
        kv_answer_release(ans);

        ans = kv_ask(handler, "select 16", strlen("select 16"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_DB_INDEX);
        kv_answer_release(ans);

        ans = kv_ask(handler, "select a", strlen("select a"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_DB_INDEX);
        kv_answer_release(ans);
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

static void _one_handler_fill(kv_handler_t *handler, int dbindex, char start)
{
        int i;
        char buf[128];
        kv_answer_t *ans;
        
        snprintf(buf, sizeof(buf), "select %d", dbindex);
        ans = kv_ask(handler, buf, strlen(buf));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        for (i = 1; i <= dbindex+1; i++) {
                snprintf(buf, sizeof(buf), "set %ckey%d value", start, i);
                _case_answer_string_equal(handler, buf, "OK");
        }
}

void case_cmd_select_one_handler()
{
        int i, j;
        char buf[128];
        kv_handler_t *handler;
        kv_answer_t *ans;

        handler = kv_create(NULL);

        for (i = 0; i < 16; i++) {
                _one_handler_fill(handler, i, 'a'+i);
        }

        /** per db size */
        for (i = 0; i < 16; i++) {
                snprintf(buf, sizeof(buf), "select %d", i); 
                ans = kv_ask(handler, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                kv_answer_release(ans);

                snprintf(buf, sizeof(buf), "%d", i+1);
                _case_answer_string_equal(handler, "dbsize", buf);
        }

        /** get per db values */
        for (i = 0; i < 16; i++) {
                snprintf(buf, sizeof(buf), "select %d", i); 
                ans = kv_ask(handler, buf, strlen(buf));
                CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
                kv_answer_release(ans);
                
                for (j = 0; j <= i; j++) {
                        snprintf(buf, sizeof(buf), "get %ckey%d", 'a'+i, j+1);
                        _case_answer_string_equal(handler, buf, "value");
                }
        }
        
        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_cmd_select_multi_handler()
{
        kv_answer_t *ans;
        kv_handler_t *handlerA, *handlerB;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);

        ans = kv_ask(handlerA, "select 5", strlen("select 5"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        ans = kv_ask(handlerB, "select 13", strlen("select 13"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "set key value", "OK");
        _case_answer_string_equal(handlerB, "lpush lkey a bb ccc", "3");

        ans = kv_ask(handlerB, "get key", strlen("get key"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        ans = kv_ask(handlerA, "lrange lkey 0 -1", strlen("lrange lkey 0 -1"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NIL);
        kv_answer_release(ans);

        _case_answer_string_equal(handlerA, "get key", "value");
        _case_answer_string_equal(handlerB, "lrange lkey 0 -1", "ccc");

        kv_destroy(handlerA);
        kv_destroy(handlerB);

        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}



#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();

        UTIL_ADD_CASE_SHORT(case_cmd_select);
        UTIL_ADD_CASE_SHORT(case_cmd_select_invalid_index);
        UTIL_ADD_CASE_SHORT(case_cmd_select_one_handler);
        UTIL_ADD_CASE_SHORT(case_cmd_select_multi_handler);
        
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
