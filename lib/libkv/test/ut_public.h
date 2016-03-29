#ifndef UT_PUBLIC_H_
#define UT_PUBLIC_H_


#include "libkv.h"
#include "ut_data_generator.h"
#include <CUnit/Basic.h>
#include <assert.h>
#include <time.h>


typedef int (*init_handler)(void);
typedef void (*case_handler)(void);


void UTIL_INIT(init_handler init, init_handler clean);
void UTIL_INIT_DEFAULT();
void UTIL_ADD_CASE(const char* desc, case_handler handler);
#define UTIL_ADD_CASE_SHORT(func) UTIL_ADD_CASE(#func "()",func)
void UTIL_RUN();
void UTIL_UNINIT();


void answer_printf(kv_answer_t *a);
void answer_printf_release(kv_answer_t *a);


void _case_answer_length_equal(kv_handler_t *handler, const char *cmd, size_t length);
void _case_answer_string_equal(kv_handler_t *handler, const char *cmd, const char *equal_str);


#endif
