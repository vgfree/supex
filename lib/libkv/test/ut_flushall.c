#include "ut_public.h"



static void _fill_amount_values(kv_handler_t *handler, int dbindex, int end)
{
        int i;
        char buf[32];

        snprintf(buf, sizeof(buf), "select %d", dbindex);
        _case_answer_string_equal(handler, buf, "OK");
        
        for (i = 1; i <= end; i++) {
                snprintf(buf, sizeof(buf), "set key%d value%d", i, i);
                _case_answer_string_equal(handler, buf, "OK");
        }
}

static void _case_cmd_flushall(kv_handler_t *handler)
{
        int i;
        char buf[32];
        kv_answer_t *ans;

        for (i = 0; i < 16; i++) {
                _fill_amount_values(handler, i, i+1);
        }

        for (i = 0; i < 16; i++) {
                snprintf(buf, sizeof(buf), "select %d", i);
                _case_answer_string_equal(handler, buf, "OK");
                snprintf(buf, sizeof(buf), "%d", i+1);
                _case_answer_string_equal(handler, "dbsize", buf);
        }

        ans = kv_ask(handler, "flushall", strlen("flushall"));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);

        for (i = 0; i < 16; i++) {
                snprintf(buf, sizeof(buf), "select %d", i);
                _case_answer_string_equal(handler, buf, "OK");
                _case_answer_string_equal(handler, "dbsize", "0");
        }
}

void case_cmd_flushall()
{
        kv_handler_t *handlerA, *handlerB, *handlerC;

        handlerA = kv_create(NULL);
        handlerB = kv_create(NULL);
        handlerC = kv_create(NULL);

        _case_cmd_flushall(handlerA);
        _case_cmd_flushall(handlerB);
        _case_cmd_flushall(handlerC);

        kv_destroy(handlerA);
        kv_destroy(handlerB);
        kv_destroy(handlerC);

        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}





#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();

        UTIL_ADD_CASE_SHORT(case_cmd_flushall);

        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
