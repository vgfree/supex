//
//  common.h
//  supex
//
//  Created by 周凯 on 15/7/16.
//  Copyright (c) 2015年 zk. All rights reserved.
//

#ifndef __supex_common_h__
#define __supex_common_h__

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

/*
 * C导出到C++
 */
#if defined(__cplusplus)
  #if !defined(__BEGIN_DECLS)
    #define __BEGIN_DECLS \
	extern "C" {
    #define __END_DECLS	\
	}
  #endif
#else
  #define __BEGIN_DECLS
  #define __END_DECLS
#endif

/*
 * intel x86 平台
 */
#if (__i386__ || __i386 || __amd64__ || __amd64)
  #ifndef __X86__
    #define __X86__
  #endif
#endif

#if defined(__X86__)
  #define _cpu_pause()  __asm__("pause")
#else
  #define _cpu_pause()  ((void)0)
#endif

#if defined(__X86__)
  #define _cpu_nop()    __asm__("nop")
#else
  #define _cpu_nop()    ((void)0)
#endif

/*
 * BOOL 类型定义
 */
#if !defined(BOOL)
  #define BOOL int
#endif

#if !defined(TRUE)
  #define TRUE (1)
#endif

#if !defined(FALSE)
  #define FALSE (0)
#endif

/*
 * 魔数
 */
#if !defined(OBJMAGIC)
  #define OBJMAGIC (0xfedcba98)
#endif

/*
 * 最小值
 */
#undef  MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*
 * 最大值
 */
#undef  MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/*
 * 计算数组长度
 */
#undef  DIM
#define DIM(x) ((int)(sizeof((x)) / sizeof((x)[0])))

/*
 * 确定范围值
 */
#undef  INRANGE
#define INRANGE(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

/*
 * 若字符串s为空则返回"NULL", 否则返回s
 */
#undef  SWITCH_NULL_STR
#define SWITCH_NULL_STR(s) ((void *)(s) == (void *)0 ? "NULL" : (s))

/*
 * 若x指针不为空则赋值
 */
#undef  SET_POINTER
#define SET_POINTER(x, v)			   \
	do { if ((x)) {				   \
		     *(x) = (__typeof__(*(x)))(v); \
	     }					   \
	} while (0)

/*
 * 返回以SIZE / BASESIZE 的最大倍数
 */
#undef  CEIL_TIMES
#define CEIL_TIMES(SIZE, BASESIZE) \
	(((SIZE) + (BASESIZE)-1) / (BASESIZE))

/*
 * 返回以BASESIZE为单位对齐的SIZE
 */
#undef  ADJUST_SIZE
#define ADJUST_SIZE(SIZE, BASESIZE) \
	(CEIL_TIMES((SIZE), (BASESIZE)) * (BASESIZE))

/*
 * 将时间设置为随机种子
 */
#define Rand() (srand((unsigned int)time(NULL)))

/*
 * 随机获取整数 x ： low <= x <= high
 */
#define RandInt(low, high)		    \
	(assert((int)(high) >= (int)(low)), \
	(int)((((double)rand()) / ((double)RAND_MAX + 1)) * (high - low + 1)) + low)

/**
 * 随机获取小数 x ： low <= x < high
 */
#define RandReal(low, high)			  \
	(assert((double)(high) >= (double)(low)), \
	(((double)rand()) / ((double)RAND_MAX + 1)) * (high - low) + low)

/**
 * 测试产生一次几率为 p(分数) 的事件
 */
#define RandChance(p) (RandReal(0, 1) < (double)(p) ? TRUE : FALSE)

/*
 * 打印调试信息
 */
#ifdef DEBUG
  #define LOG_D_VALUE   "[DEBUG]"
  #define LOG_I_VALUE   "[INFO ]"
  #define LOG_W_VALUE   "[WARN ]"
  #define LOG_F_VALUE   "[FAIL ]"
  #define LOG_E_VALUE   "[ERROR]"
  #define LOG_S_VALUE   "[SYST ]"

  #define x_printf(lgt, fmt, ...)	  \
	fprintf(stderr, LOG_##lgt##_VALUE \
		"|%16s:%4d|%20s|"	  \
		fmt, __FILE__, __LINE__,  \
		__FUNCTION__, ##__VA_ARGS__)
#else
  #define x_printf(lgt, fmt, ...) ((void)0)
#endif
#endif	/* ifndef __supex_common_h__ */

