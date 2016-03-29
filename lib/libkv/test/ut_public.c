#include "ut_public.h"


static int init_default(void)
{
        return 0;
}

static int clean_default(void)
{
        return 0;
}


static CU_pSuite util_suite = NULL;

void UTIL_INIT(init_handler init, init_handler clean)
{
    assert(CU_initialize_registry() == CUE_SUCCESS);
    assert((util_suite = CU_add_suite("Util_suite", init, clean)) != NULL);
}

void UTIL_INIT_DEFAULT()
{
    UTIL_INIT(init_default, clean_default);
}

void UTIL_ADD_CASE(const char* desc, case_handler handler)
{
    assert(CU_add_test(util_suite, desc, handler));
}

void UTIL_RUN()
{
    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
}

void UTIL_UNINIT()
{
    CU_cleanup_registry();
}

void answer_print(kv_answer_t *a)
{
        kv_answer_iter_t *iter;
        kv_answer_value_t *value;
        
        printf("answer session ===>:\n");
        if (a->errnum != ERR_NONE) {
                printf("error:%s\n", a->err);
        } else {
                iter = kv_answer_get_iter(a, ANSWER_HEAD);
                while((value = kv_answer_next(iter)) != NULL) {
                        printf("===> size:%ld value:%s\n", value->ptrlen, (char*)value->ptr);
                }
                kv_answer_release_iter(iter);
        }  
}

void answer_print_release(kv_answer_t *a)
{
        if (!a) return;
        
        answer_print(a);
        kv_answer_release(a);
}

void _case_answer_length_equal(kv_handler_t *handler, const char *cmd, size_t length)
{
        kv_answer_t *ans;

        ans = kv_ask(handler, cmd, strlen(cmd)); 
        CU_ASSERT_EQUAL(kv_answer_length(ans), length);
        kv_answer_release(ans);
}

void _case_answer_string_equal(kv_handler_t *handler, const char *cmd, const char *equal_str)
{
        kv_answer_t *ans;

        ans = kv_ask(handler, cmd, strlen(cmd));

        if (ans->errnum != ERR_NONE) {
                printf("===>err:%s\n", ans->err);
                return;
        }
  
      CU_ASSERT_STRING_EQUAL(kv_answer_value_to_string(kv_answer_first_value(ans)), equal_str);
        kv_answer_release(ans);
}
