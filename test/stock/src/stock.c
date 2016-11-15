#include <stdio.h>
#include <string.h>

#include "stock.h"

bool rule_F123(stock_trade_t *st)
{
	if ((st->opening_pice * (1 - F123_N_VALUE)) >= st->new_pice) {
		// printf("match[F123]==>%lld\n", st->timestamp);
		return true;
	} else {
		return false;
	}
}

bool rule_F121(stock_trade_t *st)
{
	return true;	// TODO
}

bool rule_F116(stock_trade_t *st)
{
	if (st->opening_pice * (1 + F116_N_VALUE) < st->max_pice) {
		if (st->max_pice * (1 - F116_M_VALUE) >= st->new_pice) {
			printf("match[F116]==>%lld\n", st->timestamp);
			return true;
		}
	}

	return false;
}

bool rule_F117(stock_trade_t *st)
{
	return true;	// TODO
}

bool rule_F118(stock_trade_t *st)
{
	return true;	// TODO
}

static struct stock_hook g_hook_maps[] = {
	{
		.rule = F123,
		.rule_impl = rule_F123,
		.mode = STOCK_BUY_IN
	},
	{
		.rule = F121,
		.rule_impl = rule_F121,
		.mode = STOCK_BUY_IN
	},
	{
		.rule = F116,
		.rule_impl = rule_F116,
		.mode = STOCK_SELL_OUT
	},
	{
		.rule = F117,
		.rule_impl = rule_F117,
		.mode = STOCK_SELL_OUT
	},
	{
		.rule = F118,
		.rule_impl = rule_F118,
		.mode = STOCK_SELL_OUT
	}
};

bool judge_one_rule(stock_trade_t *st, struct stock_hook *hook, int rules)
{
	if (st->mode != hook->mode) {
		return false;
	}

	if ((rules & hook->rule) == 0) {
		return false;
	}

	return hook->rule_impl(st);
}

void judge_match_rules(stock_trade_t *st, int rules)
{
	int     i = 0;
	int     all = sizeof(g_hook_maps) / sizeof(struct stock_hook);

	for (i = 0; i < all; i++) {
		judge_one_rule(st, &g_hook_maps[i], rules);
	}
}

