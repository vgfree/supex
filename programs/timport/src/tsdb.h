#ifndef __TSDB_H_
#define __TSDB_H_

#include "zk.h"
#include "list.h"

typedef struct _tsdb_conn
{
	struct list_head        list;
	data_node_t             dn;
} tsdb_conn_t;

typedef struct _tsdb
{
	zk_ctx_t                *zc;
	struct list_head        pool;
	AO_SpinLockT            lock;
	enum
	{
		NO_READY,
		IS_READY
	}                       status;
} tsdb_t;

tsdb_t *tsdb_open(const char *zk_servers, const char *rnode);

bool tsdb_is_ready(tsdb_t *tsdb);

void tsdb_close(tsdb_t *tsdb);

tsdb_conn_t *get_tsdb_conn(tsdb_t *tsdb, int key);
#endif	/* ifndef __TSDB_H_ */

