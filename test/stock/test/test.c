#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>

#include "stock.h"
#include "stock_shm.h"

/*
 *  microsecond
 */
long long ustime(void)
{
	struct timeval cur;

	gettimeofday(&cur, NULL);
	return cur.tv_sec * 1000000 + cur.tv_usec;
}

int main(void)
{
	bool ok = stock_shm_open();

	assert(ok == true);

	size_t  count = stock_shm_get_count();
	size_t  limit = stock_shm_get_limit();
	printf("get shm count %ld of limit %ld\n", count, limit);

	long long       sta = 0;
	long long       end = 0;
	double          use = 0;
	/*************test all use time***************/
	sta = ustime();
	size_t i = 0;

	for (i = 1; i <= count; i++) {
		stock_trade_t *one = stock_shm_pull(i);
		judge_match_rules(one, F123 | F116);
	}

	end = ustime();
	use = (end - sta) / 1000000;
	printf("\t\tTOTAL TIME USE====%0.10fs\n", use);

	/*************test F123 5000000 tick ***************/
	sta = ustime();
	i = 0;

	for (i = 0; i < 5000000; i++) {
		size_t          idx = i % count + 1;
		stock_trade_t   *one = stock_shm_pull(idx);
		judge_match_rules(one, F123);
	}

	end = ustime();
	use = (end - sta) / 1000000;
	printf("\t\tF123 5000000 tick TIME USE====%0.10fs\n", use);
	return 1;
}

