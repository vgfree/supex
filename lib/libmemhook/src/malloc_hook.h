//
//  malloc_hook.h
//  内存分配释放钩子函数声明
//
//  Created by 周凯 on 14/07.
//  Copyright (c) 2015年 zk. All rights reserved.
//

#ifndef __malloc_hook_h__
#define __malloc_hook_h__

#include "libmini.h"

__BEGIN_DECLS

/* ===================================================================
 * 内存钩子函数
 * =================================================================== */

#if defined(__LINUX__)
/* 启用内存分配、释放原始实现 */
void *__real_malloc(size_t);

void *__real_calloc(size_t, size_t);

void *__real_realloc(void *, size_t);

void __real_free(void *);

  #define real_malloc   __real_malloc
  #define real_calloc   __real_calloc
  #define real_realloc  __real_realloc
  #define real_free     __real_free
#else
  #include <jemalloc.h>
  #define real_malloc   je_malloc
  #define real_calloc   je_calloc
  #define real_realloc  je_realloc
  #define real_free     je_free
#endif

void *__wrap_malloc(size_t);

void *__wrap_calloc(size_t, size_t);

void *__wrap_realloc(void *, size_t);

void __wrap_free(void *);

/* ===================================================================
 * 其他钩子函数
 * =================================================================== */
char *__wrap_strdup(const char *);

char *__wrap_strndup(const char *, size_t);

#define Strdup  __wrap_strdup
#define Strndup __wrap_strndup

__END_DECLS
#endif	/* ifndef __malloc_hook_h__ */

