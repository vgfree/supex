/***********************************************************************
 * Copyright (C), 2010-2015,
 * ShangHai Language Mirror Automobile Information Technology Co.,Ltd.
 *
 *   File Name: xmq.h
 *      Author: liubaoan@mirrtalk.com
 *     Version: V1.0
 *        Date: 2015/12/11
 *
 *     1.
 *     2.
 *     3.
 *
 *      Others:
 *        (null)
 *
 *     History:
 *        Date:           Author:            Modification:
 *
 **********************************************************************/

#ifndef _XMQ_H_
#define _XMQ_H_

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#include "xmq_msg.h"
#include "base/xlist.h"

#define  XMQ_DBNAME_LEN         42	// 00000000000000000001-00000000000005000000
#define  XMQ_IDENTITY_LEN       32
#define  XMQ_PATH_LEN           256

typedef struct _bin_entry bin_entry_t;
typedef struct _xmq_ctx xmq_ctx_t;
typedef struct _xmq_ops xmq_ops_t;
typedef struct _xmq_db xmq_db_t;
typedef struct _xmq_producer xmq_producer_t;
typedef struct _xmq_consumer xmq_consumer_t;
typedef struct _xmq_consiter xmq_consiter_t;

/*以下函数类型由用户实现，函数实现可能因媒介不同(如leveldb、redis等)而不同*/
typedef void *(xmq_kv_ctx_init)(const char *db_name);
typedef int (xmq_kv_put)(void *kv_ctx, const bin_entry_t *keys, const bin_entry_t *values, int pairs);
typedef int (xmq_kv_get)(void *kv_ctx, const bin_entry_t *keys, bin_entry_t *values, int count);
typedef int (xmq_kv_ctx_free)(void *kv_ctx);


enum
{
	KV_LOAD_FAIL = -1,
	KV_LOAD_SUCC = 0,
	KV_LOAD_NULL = 1,
};


struct _bin_entry
{
	void    *data;
	size_t  len;
};

#define set_bin_entry(bin, ptr, size)	     \
	do {				     \
		(bin)->data = (void *)(ptr); \
		(bin)->len = (size_t)(size); \
	} while (0)

#define release_bin_entry(bin)	   \
	do {			   \
		free((bin)->data); \
		(bin)->len = 0;	   \
	} while (0)

#define bin_dup_string(b) strndup((b)->data, (b)->len)

struct _xmq_ops
{
	xmq_kv_ctx_init *kv_ctx_init;		//
	xmq_kv_put      *kv_put;		//
	xmq_kv_get      *kv_get;		//
	xmq_kv_ctx_free *kv_ctx_free;		//
};
/* */
struct _xmq_ctx
{
	char            kv_db_path[XMQ_PATH_LEN];		// The databases path, init by user transmit.
	size_t          kv_max_records;				// The maxmium of one KV database.
	void            *g_state_ctx;				// The single database handle for moniter global state.

	/* Chain list(s), to load all the states information from db "./moniter". */
	xlist_t         list_databases;		// All the been used databases. <db1,db2,..,dbN>
	xlist_t         list_producers;		// Load all the producers. <P1,P2...>
	xlist_t         list_consumers;		// Load all the consumers. <master,node1,node2.>
	xlist_t         *indicator;		// The indicator, point to '&list_consumers'.

	/* For producers. */
	xmq_db_t        *last_write_db;			// Last write K/V database object.
	uint64_t        last_write_db_seq;		// Last K/V database's wrote sequence.
	pthread_mutex_t lock_write;			// Lock for producers writting the database data[0-N].
	pthread_cond_t  lock_cond;			// The mutex condition for fetch.
	int             waiters;			// Count of waiters to fetch XMQ queue.

	/* Operater the leveldb or redis K/V database. init by user transmi. */
	xmq_ops_t       kv_ops;
};

/* */
struct _xmq_db
{
	xlist_t         node;
	uint64_t        db_start;			// database start index;
	uint64_t        db_end;				// database end   index;
	char            db_name[XMQ_DBNAME_LEN];	// "00000000000000000001-00000000000005000000"
	void            *db_handler;			// The database handle pointer, returned by xmq_ctx_init("db_name").
};

/* */
struct _xmq_producer
{
	xlist_t         node;
	char            identity[XMQ_IDENTITY_LEN];
	xmq_ctx_t       *xmq_ctx;
};

/* */
struct _xmq_consumer
{
	xlist_t         node;
	char            identity[XMQ_IDENTITY_LEN];
	char            fetch_key[XMQ_IDENTITY_LEN + 14];
	uint64_t        last_fetch_seq;
	xmq_ctx_t       *xmq_ctx;
};

/* 针对消费者创建的迭代器,主要用于main程序重启后,初始化时,
 * 重新加载之前已经正常同步的客户端(消费者)状态信息 */
struct _xmq_consiter
{
	bool            (*has_next)(xmq_ctx_t *xmq);
	xmq_consumer_t  *(*next)(xmq_ctx_t *xmq);
};

extern xmq_ctx_t *xmq_context_init(const char *kv_dbname, size_t kv_max_records,
	xmq_kv_ctx_init *kv_ctx_init,
	xmq_kv_put *kv_put,
	xmq_kv_get *kv_get,
	xmq_kv_ctx_free *kv_ctx_free);

extern void xmq_context_destroy(xmq_ctx_t *ctx);

extern int xmq_register_producer(xmq_ctx_t *ctx, const char *pid);

extern int xmq_register_consumer(xmq_ctx_t *ctx, const char *cid);

extern int xmq_unregister_producer(xmq_ctx_t *ctx, const char *pid);

extern int xmq_unregister_consumer(xmq_ctx_t *ctx, const char *cid);

extern xmq_producer_t *xmq_get_producer(xmq_ctx_t *ctx, const char *pid);

extern xmq_consumer_t *xmq_get_consumer(xmq_ctx_t *ctx, const char *cid);

extern xmq_consiter_t xmq_get_consumer_iterator(xmq_ctx_t *ctx);

extern int xmq_push_tail(xmq_producer_t *producer, const xmq_msg_t *msg);

extern xmq_msg_t *xmq_pull_that(xmq_consumer_t *consumer, uint64_t seq);

extern int xmq_update_that(xmq_consumer_t *consumer, uint64_t seq);

extern uint64_t xmq_fetch_position(xmq_consumer_t *consumer);
#endif	/* ifndef _XMQ_H_ */

