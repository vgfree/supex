#pragma once

#include <stdbool.h>
#include <stdlib.h>

typedef int (*ASYNC_OPS_VARY_INIT)(void *parser, char *const *data, unsigned const *size);
typedef int (*ASYNC_OPS_SURE_INIT)(void *parser, char *const *data, unsigned const *size, size_t todo);
typedef	int (*ASYNC_OPS_FREE)(void *parser);
typedef	int (*ASYNC_OPS_WORK)(void *parser);

/*
 * 只有读需要协议操作
 */
struct async_ops {
	int type;
	int attr;
	union {
		ASYNC_OPS_VARY_INIT vary_init;
		ASYNC_OPS_SURE_INIT sure_init;
	} init;
	ASYNC_OPS_FREE free;
	ASYNC_OPS_WORK work;
};

bool async_ops_regist(struct async_ops *ops);

struct async_ops *async_ops_lookup(int type);
