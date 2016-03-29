#pragma once

#include "leveldb/c.h"
#include "xmq.h"

typedef struct ldb_pvt_struct
{
	leveldb_t               *db;
	leveldb_options_t       *options;
	leveldb_readoptions_t   *roptions;
	leveldb_writeoptions_t  *woptions;
} ldb_pvt_t;

extern ldb_pvt_t *ldb_pvt_create(const char *path);

extern int ldb_pvt_destroy(void *ldb_pvt);

extern int driver_ldb_put(void *ldb_pvt, binary_entry_t *keys, binary_entry_t *values, unsigned int pairs);

extern int driver_ldb_get(void *ldb_pvt, binary_entry_t *keys, binary_entry_t *values, unsigned int count);

