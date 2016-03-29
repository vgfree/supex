//
//  common_test.c
//  mylib
//
//  Created by 周凯 on 14/12/19.
//  Copyright (c) 2014年 zk. All rights reserved.
//

#include "common_test.h"
#include <pthread.h>

__thread
struct timeval start_time_stat;

__thread
struct timeval end_time_stat;

// static __thread
AO_T statTotal_s[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
// static __thread
AO_T statTotal_ms[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
// static __thread
AO_T statTotal_us[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static const
char *unit_s = " s";
static const
char *unit_ms = "ms";
static const
char *unit_us = "us";

static const
int point1 = 3;
static const
int point2 = 7;
static const
int point3 = 10;
static const
char *printFmt = ">>> [%3d, %4d) %s :%d\n";

__thread
long maxTm = 0;
__thread
long minTm = -1U;

volatile
long totalTm = 0;

static
AO_SpinLockT lock = { OBJMAGIC, 0, 0 };

static inline
void _cmpTotal(long tm, AO_T resulte[])
{
	if (tm < point1) {
		ATOMIC_INC((AO_T *)&resulte[0]);
	} else if (tm < point2) {
		ATOMIC_INC((AO_T *)&resulte[1]);
	} else if (tm < point3) {
		ATOMIC_INC((AO_T *)&resulte[2]);
	} else if (tm / 10 < point1) {
		ATOMIC_INC((AO_T *)&resulte[3]);
	} else if (tm / 10 < point2) {
		ATOMIC_INC((AO_T *)&resulte[4]);
	} else if (tm / 10 < point3) {
		ATOMIC_INC((AO_T *)&resulte[5]);
	} else if (tm / 100 < point1) {
		ATOMIC_INC((AO_T *)&resulte[6]);
	} else if (tm / 100 < point2) {
		ATOMIC_INC((AO_T *)&resulte[7]);
	} else if (tm / 100 < point3) {
		ATOMIC_INC((AO_T *)&resulte[8]);
	}
}

static inline
void _printTotalResult(const char       *unit,
	const AO_T                      total[])
{
	fprintf(stderr, printFmt, 0, point1, unit, total[0]);
	fprintf(stderr, printFmt, point1, point2, unit, total[1]);
	fprintf(stderr, printFmt, point2, point3, unit, total[2]);

	fprintf(stderr, printFmt, 10, point1 * 10, unit, total[3]);
	fprintf(stderr, printFmt, point1 * 10, point2 * 10, unit, total[4]);
	fprintf(stderr, printFmt, point2 * 10, point3 * 10, unit, total[5]);

	fprintf(stderr, printFmt, 100, point1 * 100, unit, total[6]);
	fprintf(stderr, printFmt, point1 * 100, point2 * 100, unit, total[7]);
	fprintf(stderr, printFmt, point2 * 100, point3 * 100, unit, total[8]);
}

void StatAndPrint(const char *msg)
{
	long    tm = 0;
	long    us = 0;
	long    ms = 0;
	long    s = 0;

	tm = end_time_stat.tv_sec * 1000000 + end_time_stat.tv_usec -
		(start_time_stat.tv_sec * 1000000 + start_time_stat.tv_usec);

	maxTm = MAX(maxTm, tm);
	minTm = MIN(minTm, tm);

	ATOMIC_ADD((AO_T *)&totalTm, tm);

	s = tm / 1000000;
	ms = tm % 1000000 / 1000;
	us = tm % 1000000 % 1000;

	if ((us > 0) && (s == 0) && (ms == 0)) {
		_cmpTotal(us, statTotal_us);
	} else if ((ms > 0) && (s == 0)) {
		_cmpTotal(ms, statTotal_ms);
	} else if (s > 0) {
		_cmpTotal(s, statTotal_s);
	} else {
		_cmpTotal(us, statTotal_us);
	}

#if defined _STAT_DETAIL
	char    buff[128] = { 0 };
	size_t  len = 0;
	len = snprintf(buff, sizeof(buff),
			"%-40s escape time : %03ld.%03ld.%03ld(s.ms.us)\n",
			SWITCH_NULL_STR(msg),
			tm / 1000000,
			tm % 1000000 / 1000,
			tm % 1000000 % 1000);

	write(STDOUT_FILENO, buff, MIN(len, sizeof(buff) - 1));
#endif
}

void (PrintResult)(const char *name)
{
	AO_SpinLock(&lock);

	fprintf(stderr, "==========================================\n");
	fprintf(stderr, "test name [ %s ] , thread id :%p\n",
		SWITCH_NULL_STR(name), (void *)pthread_self());
	fprintf(stderr, "==========================================\n");

	_printTotalResult(unit_us, statTotal_us);
	_printTotalResult(unit_ms, statTotal_ms);
	_printTotalResult(unit_s, statTotal_s);

	minTm = ((long)minTm) == -1U ? 0 : minTm;

	fprintf(stderr, "        min escape time : %03ld.%03ld.%03ld(s.ms.us)\n",
		minTm / 1000000,
		minTm % 1000000 / 1000,
		minTm % 1000000 % 1000);

	fprintf(stderr, "        max escape time : %03ld.%03ld.%03ld(s.ms.us)\n",
		maxTm / 1000000,
		maxTm % 1000000 / 1000,
		maxTm % 1000000 % 1000);

	fprintf(stderr, "  escape time currently : %03ld.%03ld.%03ld(s.ms.us)\n",
		totalTm / 1000000,
		totalTm % 1000000 / 1000,
		totalTm % 1000000 % 1000);

	fprintf(stderr, "==========================================\n");

	AO_SpinUnlock(&lock);
}

