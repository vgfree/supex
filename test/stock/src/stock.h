#pragma once

#include <stdbool.h>

typedef enum _TRADE_TYPE
{
	STOCK_BUY_IN = 0,
	STOCK_SELL_OUT,
} TRADE_TYPE_T;

typedef struct stock_trade
{
	TRADE_TYPE_T    mode;

	long long       timestamp;		/*行情原始时间*/
	double          yesterday_last_pice;	/*昨收价*/
	double          opening_pice;		/*开盘价*/
	double          max_pice;		/*最高价*/
	double          min_pice;		/*最低价*/
	double          new_pice;		/*最新价*/
} stock_trade_t;

typedef struct stock_hook
{
	int             rule;
	bool (*rule_impl)(stock_trade_t *st);
	TRADE_TYPE_T    mode;
} stock_hook_t;

#define F123            (1 << 0)
#define F121            (1 << 1)
#define F116            (1 << 2)
#define F117            (1 << 3)
#define F118            (1 << 4)

#define F123_N_VALUE    (0.02)

#define F116_N_VALUE    (0.08)
#define F116_M_VALUE    (0.03)

void judge_match_rules(stock_trade_t *st, int rules);

