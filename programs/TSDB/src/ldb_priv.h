#pragma once

#include "ldb.h"

#define SOME_KV_NODES_COUNT 1024

struct kv_node
{
	int     klen;
	int     vlen;
	char    *key;
	char    *val;
};

struct some_kv
{
	struct kv_node  nodes[SOME_KV_NODES_COUNT];
	struct some_kv  *next;
	struct some_kv  *prev;
};

struct kv_list
{
	int             count;
	int             klens;
	int             vlens;
	int             knubs;
	int             vnubs;
	struct some_kv  head;
};

/*
 * Get a key range info.
 */
char *ldb_lrangeget(struct _leveldb_stuff *ldbs, const char *key, size_t suf_klen, const char *st_time, size_t st_tlen, const char *ed_time, size_t ed_tlen, int *size);

/*
 * Get keys.
 * Now just support prefix match.
 */
char *ldb_keys(struct _leveldb_stuff *ldbs, const char *ptn, size_t ptn_len, int *size);

/*
 * Get values.
 * Regular match.
 * XXX: Please use caution.
 */
char *ldb_values(struct _leveldb_stuff *ldbs, const char *ptn, size_t ptn_len, int *size);

/*
 * Get server info.
 */
char *ldb_info(struct _leveldb_stuff *ldbs, int *size);

