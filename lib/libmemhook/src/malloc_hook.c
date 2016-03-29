/*
 * 功能：当使用-Wl,-wrap,xxx链接该库时，所有的xxx函数将被__wrap_xxx替换
 * 而之前的xxx函数被重定向到__real_xxx函数指针中，故必须实现__wrap_xxx
 * 函数
 * Created by 周凯 on 14/07.
 * Copyright (c) 2015年 zk. All rights reserved.
 */
// #undef DEBUG

#include <execinfo.h>
#include <pthread.h>

#include "malloc_hook.h"
// #include "utils.h"
#include "node_manager.h"

#include <assert.h>

// #define STACK_START             (2)
static __thread int STACK_START = (2);
#define STACK_MAX (8)

/*
 * 分配内存以机器字节大小对齐
 * 减少碎片
 */
#define aligememorysize(size) \
	(ADJUST_SIZE(size, sizeof(long)))

#define assertalign(ptr) \
	((((unsigned long)(ptr) % sizeof(long)) == 0))

#ifdef MALLOC_HOOK_MZERO
  #define MZERO(x, y)   (memset((x), 0, (y)))
#else
  #define MZERO(x, y)   ((void)0)
#endif

/* ===================================================================
 * 节点管理函数
 * =================================================================== */

static
void AddStackInfo(void *ptr, long size, int start);

static
void DelStackInfo(void *ptr, int start);

/* ===================================================================
 * 内存操作钩子函数
 * =================================================================== */
void *__wrap_malloc(size_t size)
{
	size = aligememorysize(size);

	assert(size);

	void *ptr = real_malloc(size);

	if (ptr) {
		MZERO(ptr, size);
		AddStackInfo(ptr, size, STACK_START);
	}

	return ptr;
}

void *__wrap_calloc(size_t n, size_t size)
{
	size = aligememorysize(size * n);

	assert(size);

	void *ptr = real_calloc(1, size);

	if (ptr) {
		MZERO(ptr, size);
		AddStackInfo(ptr, size, STACK_START);
	}

	return ptr;
}

void *__wrap_realloc(void *old, size_t size)
{
	size_t  oldsize = 0;
	void    *ptr = NULL;

	size = aligememorysize(size);

	assert(size);

	if (!PullManager(old, (long *)&oldsize)) {
#ifdef MALLOC_HOOK_CHECK
		if (old) {
			abort();
		}
#endif
		ptr = real_realloc(old, size);
	} else {
		if (size == 0) {
			ptr = NULL;
			/*一定要先删除节点，再释放原来的内存*/
			DelStackInfo(old, STACK_START);
			real_free(old);
		} else {
			ptr = real_calloc(1, size);

			if (ptr) {
				MZERO(ptr, size);
				memcpy(ptr, old, MIN(size, oldsize));
				/*一定要先删除节点，再释放原来的内存*/
				DelStackInfo(old, STACK_START);
				real_free(old);
			}
		}
	}

	if (ptr) {
		AddStackInfo(ptr, size, STACK_START);
	}

	return ptr;
}

void __wrap_free(void *ptr)
{
	if (!ptr) {
		return;
	}

	DelStackInfo(ptr, STACK_START);
	real_free(ptr);
}

/* ===================================================================
 * 其他钩子函数
 * =================================================================== */
char *__wrap_strdup(const char *str)
{
	char    *ptr = NULL;
	size_t  n = 0;

	if (!str) {
		/* code */
		return NULL;
	}

	n = strlen(str);

	STACK_START++;
	NewArray(n + 1, ptr);
	STACK_START--;

	if (!ptr) {
		return NULL;
	}

	snprintf(ptr, n + 1, "%s", str);

	return ptr;
}

char *__wrap_strndup(const char *str, size_t n)
{
	char *ptr = NULL;

	if (!str) {
		/* code */
		return NULL;
	}

	STACK_START++;
	NewArray(n + 1, ptr);
	STACK_START--;

	if (!ptr) {
		return NULL;
	}

	snprintf(ptr, n + 1, "%s", str);

	return ptr;
}

/* ===================================================================
 * 节点管理函数
 * =================================================================== */

/* --------------------------           */
// #defined DEBUG
#undef x_printf
#if defined(DEBUG)
  #define x_printf(lgt, fmt, ...)      \
	fprintf(stdout,		       \
		LOG_##lgt##_COLOR      \
		LOG_##lgt##_LEVEL      \
		LOG_##lgt##_COLOR      \
		fmt PALETTE_NULL "\n", \
		##__VA_ARGS__)
#elif defined(TRACE)

  #define x_printf(lgt, fmt, ...) \
	dprintf(gmhookTraceLogFD, \
		LOG_##lgt##_LEVEL \
		fmt "\n", ##__VA_ARGS__)
#else
  #define x_printf(lgt, fmt, ...)
#endif

/* --------------------------           */
// #define _GNU_SOURCE
#ifdef _GNU_SOURCE
  #include <dlfcn.h>

typedef Dl_info BtInfoT;

  #if defined(DEBUG) || defined(TRACE)
    #define BT_DATA				   \
	const char *file = NULL;		   \
	const char      *func = NULL;		   \
	const char      *ptr1 = NULL;		   \
	const char      *ptr2 = NULL;		   \
	ptr1 = SWITCH_NULL_STR(_btptr->dli_sname); \
	ptr2 = strrchr(ptr1, '/');		   \
	func = ptr2 ? ptr2 + 1 : ptr1;		   \
	ptr1 = SWITCH_NULL_STR(_btptr->dli_fname); \
	ptr2 = strrchr(ptr1, '/');		   \
	file = ptr2 ? ptr2 + 1 : ptr1;
  #else
    #define BT_DATA \
	((void)0)
  #endif

  #define PrintBackTrace(id, oper, stackinfo, start, ptr, size, btinfo)	\
	do {								\
		BtInfoT *_btptr = (btinfo);				\
		if (((*start) >= DIM((stackinfo))) ||			\
			dladdr((stackinfo)[*(start)], _btptr) == 0) {	\
			break;						\
		}							\
		if (!_btptr->dli_sname) {				\
			(*(start))++;					\
			continue;					\
		}							\
		BT_DATA;						\
		x_printf(D, "|%3d|T:%10s|TID:%8ld|PN:%20s|"		\
			"FC:%20s(%20p)|AD:%20p(%12ld)",			\
			(id), (oper),					\
			GetThreadID(),					\
			file, func,					\
			_btptr->dli_saddr,				\
			(ptr), (size));					\
		break;							\
	} while (1)
#else

typedef char BtInfoT[1];

  #define PrintBackTrace(id, oper, stackinfo, start, ptr, size, btinfo)	\
	do {								\
		x_printf(D, "|%3d|T:%10s|TID:%8ld|AD:%20p(%12ld)",	\
			(id), (oper),					\
			GetThreadID(),					\
			(ptr), (size));					\
	} while (0)
#endif	/* ifdef _GNU_SOURCE */

static
void AddStackInfo(void *ptr, long size, int start)
{
	void    *stackinfo[STACK_MAX] = {};
	BtInfoT btinfo = {};
	int     layer = 0;

	layer = backtrace(stackinfo, DIM(stackinfo));
	assert(layer > 0);

	start = INRANGE(start, STACK_START, layer - 1);

	PrintBackTrace(start, "Allocate", stackinfo, &start,
		ptr, size, &btinfo);

	if (!PushManager(ptr, size, &btinfo)) {
		x_printf(E, "Push node-manager fail :"
			" address %p size %ld\n", ptr, size);
#ifdef MALLOC_HOOK_CHECK
		abort();
#endif
	}

#ifdef DEBUG
	int i = 0;

	for (i = start + 1; i < layer; ++i) {
		/* code */
		PrintBackTrace(i, "Allocate", stackinfo, &i,
			ptr, size, &btinfo);
	}
#endif
}

static
void DelStackInfo(void *ptr, int start)
{
	void    *stackinfo[STACK_MAX] = {};
	BtInfoT btinfo = {};
	int     layer = 0;
	int     i = 0;
	long    size = 0;

	layer = backtrace(stackinfo, DIM(stackinfo));
	assert(layer > 0);

	start = INRANGE(start, STACK_START, layer - 1);

	if (!assertalign(ptr)) {
		PrintBackTrace(start, "Memory isn't allocted", stackinfo, &start,
			ptr, size, &btinfo);
#ifdef MALLOC_HOOK_CHECK
		abort();
#endif
		goto tracestack;
	}

	if (!PopManager(ptr, &size, NULL)) {
		PrintBackTrace(start, "Memory isn't allocted", stackinfo, &start,
			ptr, size, &btinfo);
#ifdef MALLOC_HOOK_CHECK
		abort();
#endif
		goto tracestack;
	}

	PrintBackTrace(start, "Free", stackinfo, &start,
		ptr, size, &btinfo);

tracestack:

#ifdef DEBUG
	for (i = start + 1; i < layer; ++i) {
		/* code */
		PrintBackTrace(i, "Free", stackinfo, &i,
			ptr, size, &btinfo);
	}
#endif
	_cpu_nop();
}

