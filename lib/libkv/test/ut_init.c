#include "ut_public.h"



void case_init()
{
        kv_handler_t *handler;
        
        handler = kv_create(NULL);
        
        CU_ASSERT_PTR_NOT_EQUAL(handler, NULL);
        kv_destroy(handler);

        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_init_after_used()
{
        kv_handler_t *handler;
        kv_answer_t *ans;
        kv_answer_iter_t *iter;
        kv_answer_value_t *value;
        const char* cmd = "set init_test_key init_test_value";

        handler = kv_create(NULL);

        ans = kv_ask(handler, cmd, strlen(cmd));
        CU_ASSERT_EQUAL(ans->errnum, ERR_NONE);
        CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), "OK");
        kv_answer_release(ans);

        kv_destroy(handler);
        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}

void case_init_stress()
{
        int i;
        kv_handler_t *handler[5000];

        for (i = 0; i < 5000; i++) {
                handler[i] = kv_create(NULL);
                CU_ASSERT_PTR_NOT_EQUAL(handler[i], NULL);
        }

        for (i = 0; i < 5000; i++) {
                kv_destroy(handler[i]);
        }

        CU_ASSERT_EQUAL(kv_get_used_memory(), 0);
}


#ifndef ENABLE_AUTO
int main(int argc, char *argv[])
{
        UTIL_INIT_DEFAULT();
        UTIL_ADD_CASE_SHORT(case_init);
        UTIL_ADD_CASE_SHORT(case_init_after_used);
        UTIL_ADD_CASE_SHORT(case_init_stress);
        UTIL_RUN();
        UTIL_UNINIT(); 

        return 0;
}
#endif
