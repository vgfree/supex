//
//  common_test.h
//  mylib
//
//  Created by 周凯 on 14/12/19.
//  Copyright (c) 2014年 zk. All rights reserved.
//

#ifndef __bLib__common_test__
#define __bLib__common_test__

#include "libmini.h"
#include <sys/time.h>

__BEGIN_DECLS

/*
 * 是否统计时间信息，编译选项
 */
#define _STAT_TIME 1

// #define _STAT_DETAIL    1

extern __thread struct timeval start_time_stat;

extern __thread struct timeval end_time_stat;

#if defined(_STAT_TIME)
  #define StartStat() \
	gettimeofday(&start_time_stat, NULL)
  #define EndStat(msg) \
	(gettimeofday(&end_time_stat, NULL), StatAndPrint(msg))
  #define PrintResult(x) \
	((PrintResult)((x)))
#else
  #define StartStat()
  #define EndStat(msg)
  #define PrintResult(x)
#endif

void StatAndPrint(const char *msg);

void (PrintResult)(const char *name);

__END_DECLS
#endif	/* defined(__bLib__common_test__) */

