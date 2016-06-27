#include "xmq.h"
#include "xmq_csv.h"
#include "libmini.h"
#include "base/utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>

///////////////////////////////////////////////////////////////////////

/* /usr/include/stdint.h
 *
 * # define UINT64_MAX     (__UINT64_C(18446744073709551615))
 *
 * Limit of `size_t' type.
 * # if __WORDSIZE == 64
 *    typedef unsigned long int   uint64_t;
 * #  define SIZE_MAX      (18446744073709551615UL)
 * # else
 *    typedef unsigned long long int  uint64_t;
 * #  define SIZE_MAX      (4294967295U)
 * # endif
 *
 * */
#define XMQ_KEY_LEN     21		// "18446744073709551615" len=20, 1 for '\0'
#ifndef ULLONG_MAX
  #define XMQ_U64_MAX   (18446744073709551615UL)
#else
  #define XMQ_U64_MAX   (ULLONG_MAX)
#endif
enum xmq_type
{
	XMQ_TYPE_DATABASE = 1,
	XMQ_TYPE_PRODUCER = 2,
	XMQ_TYPE_CONSUMER = 3,
};

#define XMQ_DATABASES           "XMQ_DATABASES"
#define XMQ_PRODUCERS           "XMQ_PRODUCERS"
#define XMQ_CONSUMERS           "XMQ_CONSUMERS"
// #define XMQ_DATABASE_WITCH     "XMQ_DATABASE_WITCH"
// #define XMQ_DATABASE_INDEX     "XMQ_DATABASE_INDEX"
#define XMQ_DATABASE_WRITEPOS   "XMQ_DATABASE_WRITEPOS"

///////////////////////////////////////////////////////////////////////
#undef  freeif
#define freeif(p)		  \
	do {			  \
		if (p) {	  \
			free(p);  \
			p = NULL; \
		}		  \
	} while (0)

#undef  return_if_false
#define return_if_false(v, r)						\
	do {								\
		if (!(v)) {						\
			x_printf(D, "expression [ " #v " ] is false.");	\
			return (r);					\
		}							\
	} while (0)
///////////////////////////////////////////////////////////////////////

/* __get_KV_dbname() get K/V database name, [1,5000],[5001,10000] -> offset = 4999 */
static void __get_KV_dbname(char dest[XMQ_DBNAME_LEN], uint64_t start, size_t offset);

/* string append with [first,second,third] by split. */
static char *__string_append_3rd(const char *prefix, const char *split, const char *first, const char *second, const char *third);

/* Delete the substr, and return the left string. */
static char *__string_delete_by_substr(char *src, const char *substr, const char *split);

/* Append string 'append' and rewrite to database. */
static int __write_back_append(xmq_ctx_t *ctx, const char *key, const char *append, const char *split);

/* Delete string 'delete' and rewrite to database. */
static int __write_back_delete(xmq_ctx_t *ctx, const char *key, const char *delete, const char *split);

/* The base function to operator the K/V database. */
static int __load_int(xmq_ops_t *ops, void *kv_ctx, const char *key, int *value);

static int __write_int(xmq_ops_t *ops, void *kv_ctx, const char *key, int value);

static int __load_uint64(xmq_ops_t *ops, void *kv_ctx, const char *key, uint64_t *value);

static int __write_uint64(xmq_ops_t *ops, void *kv_ctx, const char *key, uint64_t value);

static int __load_string_bin(xmq_ops_t *ops, void *kv_ctx, const char *key, void **value, size_t *len);

static int __write_string_bin(xmq_ops_t *ops, void *kv_ctx, const char *key, const void *value, size_t len);

/* The main function to operator the K/V database. */
static int __write_string_string(xmq_ctx_t *ctx, const char *key, const char *value);

/* Loading such as "P1,P2,P3,..PN to list_XXX." */
static int __load_string_list(xmq_ctx_t *ctx, const char *key, xlist_t *head, enum xmq_type type);

static int __list_to_csv(xlist_t *head, csv_parse_t *csv, int type);

static int __csv_to_list(xmq_ctx_t *ctx, csv_parse_t *csv, xlist_t *head, int type);

/* Only open the K/V database and load it to list_database or delete from it.*/
static int __load_database(xmq_ctx_t *ctx, const char *name);

static int __unload_database(xmq_ctx_t *ctx, const char *name);

static int __open_database(xmq_ctx_t *ctx, uint64_t last_writepos);

/* Loading all the resources was called by xmq_context_init. */
static int __load_all_resouces(xmq_ctx_t *ctx);

/* Check the key and return it with type {xmq_db_t|xmq_producer_t|xmq_consumer_t}. */
static void *__is_obj_exist(xmq_ctx_t *ctx, const char *key, int type);

/* Get the K/V database handle pointer by a index.
 * This function will iterator the list_database by reverse. */
static xmq_db_t *__get_db_handler(xmq_ctx_t *ctx, uint64_t fetchpos);

#define key_of_consumer_fetchpos(c) __string_append_3rd("XMQ", "_", (c), "FETCHPOS", NULL)

///////////////////////////////////////////////////////////////////////

xmq_ctx_t *xmq_context_init(const char *kv_db_path, size_t kv_max_records,
	xmq_kv_ctx_init *kv_ctx_init,
	xmq_kv_put *kv_put,
	xmq_kv_get *kv_get,
	xmq_kv_ctx_free *kv_ctx_free)
{
	return_if_false((kv_db_path && kv_max_records > 0 && kv_ctx_init && kv_put && kv_get && kv_ctx_free), NULL);

	assert(XMQ_KEY_LEN == 21);
	assert(XMQ_DBNAME_LEN == 42);
	/* You should never change calloc to malloc. calloc will clear memory with '\0'*/
	xmq_ctx_t *ctx = (xmq_ctx_t *)calloc(1, sizeof(xmq_ctx_t));
	return_if_false(ctx, NULL);

	/* fill setting*/
	strncpy(ctx->kv_db_path, kv_db_path, XMQ_PATH_LEN - 1);
	ctx->kv_max_records = kv_max_records;
	ctx->kv_ops.kv_ctx_init = kv_ctx_init;
	ctx->kv_ops.kv_put = kv_put;
	ctx->kv_ops.kv_get = kv_get;
	ctx->kv_ops.kv_ctx_free = kv_ctx_free;

	/* check */
	if (mkdir_if_doesnt_exist(kv_db_path)) {
		x_printf(E, "mkdir_if_doesnt_exist: Execute fail.");
		free(ctx); return NULL;
	}

	list_init(&ctx->list_databases);
	list_init(&ctx->list_consumers);
	list_init(&ctx->list_producers);

	pthread_mutex_init(&ctx->lock_write, NULL);
	pthread_cond_init(&ctx->lock_cond, NULL);

	if (__load_all_resouces(ctx)) {
		x_printf(E, "__load_all_resouces fail. Initialize fail.");
		free(ctx); return NULL;
	}

	return ctx;
}

void xmq_context_destroy(xmq_ctx_t *ctx)
{
	if (ctx != NULL) {
		xlist_t *iter;
		list_foreach(iter, &ctx->list_producers)
		{
			list_del(iter);
			xmq_producer_t *producer = container_of(iter, xmq_producer_t, node);
			free(producer);
		}
		list_foreach(iter, &ctx->list_consumers)
		{
			list_del(iter);
			xmq_consumer_t *consumer = container_of(iter, xmq_consumer_t, node);
			free(consumer);
		}
		list_foreach(iter, &ctx->list_databases)
		{
			list_del(iter);
			xmq_db_t *db = container_of(iter, xmq_db_t, node);
			ctx->kv_ops.kv_ctx_free(db->db_handler);
			free(db);
		}
		ctx->kv_ops.kv_ctx_free(ctx->g_state_ctx);

		pthread_mutex_destroy(&ctx->lock_write);
		pthread_cond_destroy(&ctx->lock_cond);
		free(ctx);
	}
}

int xmq_register_producer(xmq_ctx_t *ctx, const char *pid)
{
	return_if_false((ctx && pid), -1);

	/* Checksum, if it doesn't exist, create it and load it to list_producers.*/
	if (!__is_obj_exist(ctx, pid, XMQ_TYPE_PRODUCER)) {
		x_printf(I, "Producer [%s] doesn't exist. we'll register it.", pid);

		/* Create producer update state to db './kvstate'*/
		xmq_producer_t *producer = (xmq_producer_t *)calloc(1, sizeof(xmq_producer_t));
		return_if_false(producer, -1);

		strncpy(producer->identity, pid, sizeof(producer->identity) - 1);
		producer->xmq_ctx = ctx;

		/* Update the XMQ_PRODUCERS */
		if (__write_back_append(ctx, XMQ_PRODUCERS, producer->identity, ",")) {
			x_printf(W, "__write_back_append(Key:%s Value:[%s]) fail", XMQ_PRODUCERS, producer->identity);
			free(producer);
			return -1;
		}

		x_printf(I, "__write_back_append(Key:%s Value:[%s]), Register succeed.", XMQ_PRODUCERS, producer->identity);

		list_init(&producer->node);
		list_add_tail(&producer->node, &ctx->list_producers);
	} else {
		x_printf(W, "Producer [%s] has already registered and loaded successful.", pid);
		return 1;
	}

	return 0;
}

/*先写入消费者的last_seq 再追加消费者key string list*/
int xmq_register_consumer(xmq_ctx_t *ctx, const char *cid)
{
	return_if_false((ctx && cid), -1);

	/* Checksum, if it doesn't exist, then create it and load it to list_consumers.*/
	if (!__is_obj_exist(ctx, cid, XMQ_TYPE_CONSUMER)) {
		x_printf(I, "consumer [%s] doesn't exist. we'll register it.", cid);

		/* Create consumer read state to db './kvstate' and add to list_consumers */
		xmq_consumer_t *consumer = (xmq_consumer_t *)calloc(1, sizeof(xmq_consumer_t));
		return_if_false(consumer, -1);

		strncpy(consumer->identity, cid, sizeof(consumer->identity) - 1);
		consumer->xmq_ctx = ctx;
		consumer->last_fetch_seq = 1;

		char *key_fetchpos = key_of_consumer_fetchpos(consumer->identity);
		strcpy(consumer->fetch_key, key_fetchpos);
		free(key_fetchpos);

		if (__write_uint64(&ctx->kv_ops, ctx->g_state_ctx, consumer->fetch_key, consumer->last_fetch_seq)) {
			x_printf(E, "__write_uint64(Key:%s Value:[%llu]) fail", consumer->fetch_key, consumer->last_fetch_seq);
			free(consumer);
			return -1;
		}

		// Append this consumer to XMQ_CONSUMERS in db kvstate, and set relevant macro fields. see: key_of_XXX.
		if (__write_back_append(ctx, XMQ_CONSUMERS, consumer->identity, ",")) {
			x_printf(E, "__write_back_append(Key:%s Value:[%s]) fail", XMQ_CONSUMERS, consumer->identity);
			free(consumer);
			return -1;
		}

		x_printf(I, "__write_back_append(Key:%s Value:[%s]), Register succeed.", XMQ_CONSUMERS, consumer->identity);

		list_init(&consumer->node);
		list_add_tail(&consumer->node, &ctx->list_consumers);
	} else {
		x_printf(W, "consumer [%s] has already register. cann't register again.", cid);
		return 1;
	}

	return 0;
}

xmq_producer_t *xmq_get_producer(xmq_ctx_t *ctx, const char *pid)
{
	return (xmq_producer_t *)__is_obj_exist(ctx, pid, XMQ_TYPE_PRODUCER);
}

xmq_consumer_t *xmq_get_consumer(xmq_ctx_t *ctx, const char *cid)
{
	return (xmq_consumer_t *)__is_obj_exist(ctx, cid, XMQ_TYPE_CONSUMER);
}

static bool has_next(xmq_ctx_t *xmq)
{
	return xmq->indicator->next != &xmq->list_consumers;
}

static xmq_consumer_t *next(xmq_ctx_t *xmq)
{
	xmq->indicator = xmq->indicator->next;

	return (xmq_consumer_t *)container_of(xmq->indicator, xmq_consumer_t, node);
}

xmq_consiter_t xmq_get_consumer_iterator(xmq_ctx_t *xmq)
{
	xmq->indicator = &xmq->list_consumers;

	xmq_consiter_t xiter = { .next = next, .has_next = has_next };

	return xiter;
}

int xmq_unregister_producer(xmq_ctx_t *ctx, const char *pid)
{
	return_if_false((ctx && pid), -1);

	xmq_producer_t *producer = (xmq_producer_t *)__is_obj_exist(ctx, pid, XMQ_TYPE_PRODUCER);

	if (producer) {
		list_del(&producer->node);

		/* Reset the state of this producer in db kvstate. */
		if (__write_back_delete(ctx, XMQ_PRODUCERS, producer->identity, ",")) {
			x_printf(W, "__write_back_delete: Update the db ./kvstate with KEY:%s VAL:%s fail.", XMQ_PRODUCERS, producer->identity);
			free(producer);
			return -1;
		}

		free(producer);
		x_printf(I, "Producer [%s] was unregistered succeed.", pid);
	}

	return 0;
}

int xmq_unregister_consumer(xmq_ctx_t *ctx, const char *cid)
{
	return_if_false((ctx && cid), -1);

	xmq_consumer_t *consumer = (xmq_consumer_t *)__is_obj_exist(ctx, cid, XMQ_TYPE_CONSUMER);

	if (consumer) {
		list_del(&consumer->node);

		/* Reset the state of this consumer in db kvstate. */
		if (__write_back_delete(ctx, XMQ_CONSUMERS, consumer->identity, ",")) {
			x_printf(W, "__write_back_delete: Update the db ./kvstate with KEY:%s fail.", XMQ_CONSUMERS);
			free(consumer);
			return -1;
		}

		free(consumer);
		x_printf(I, "Consumer [%s] was unregistered succeed.", cid);
	}

	return 0;
}

int xmq_push_tail(xmq_producer_t *producer, const xmq_msg_t *msg)
{
	return_if_false((producer && msg), -1);

	xmq_ctx_t *ctx = producer->xmq_ctx;

	pthread_mutex_lock(&ctx->lock_write);

	uint64_t seq = ctx->last_write_db_seq;

	/* 1. Checking: uint64_t is full with sequence = XMQ_U64_MAX */
	if (seq == XMQ_U64_MAX) {
		pthread_mutex_unlock(&ctx->lock_write);
		x_printf(S, "The last write sequence is 18446744073709551615, will stop push any message.");
		return -1;
	}

	/* 2. Checking: One DB was write full!
	 * Create a new database to store K/V data, load it to list_database. */
	if ((seq % ctx->kv_max_records) == 1) {
		if (__open_database(ctx, seq)) {
			pthread_mutex_unlock(&ctx->lock_write);
			x_printf(W, "__open_database(last_writepos:%llu) fail.", seq);
			return -1;
		}
	}

	/* Write new K/V data to database [0~N]. */
	char key[XMQ_KEY_LEN];
	snprintf(key, sizeof(key), "%020llu", seq);

	if (__write_string_bin(&ctx->kv_ops, ctx->last_write_db->db_handler, key, msg, xmq_msg_total_size(msg))) {
		pthread_mutex_unlock(&ctx->lock_write);
		x_printf(F, "__write_string_bin(key:%s, value:%s) fail.", key, (char *)msg->data);
		return -1;
	}

	/* Update XMQ_DATABASE_WRITEPOS */
	if (__write_uint64(&ctx->kv_ops, ctx->g_state_ctx, XMQ_DATABASE_WRITEPOS, seq + 1)) {
		pthread_mutex_unlock(&ctx->lock_write);
		x_printf(F, "__write_uint64(key:%s, value:%lld) fail.", XMQ_DATABASE_WRITEPOS, seq + 1);
		return -1;
	}

	ctx->last_write_db_seq++;

	pthread_mutex_unlock(&ctx->lock_write);

	return 0;
}

xmq_msg_t *xmq_pull_that(xmq_consumer_t *consumer, uint64_t seq)
{
	assert(consumer);

	/* 1. Checking: uint64_t is full with sequence = XMQ_U64_MAX */
	if (seq == XMQ_U64_MAX) {
		x_printf(S, "Sequence of you request is 18446744073709551615 or (uint64_t)-1, we'll stop fetch next message.");
		return NULL;
	}

	xmq_ctx_t       *ctx = consumer->xmq_ctx;
	xmq_db_t        *db = __get_db_handler(ctx, seq);

	if (db == NULL) {
		x_printf(W, "__get_db_handler(fetchpos:%llu) fail, database doesn't exist.", seq);
		return NULL;
	}

	char key[XMQ_KEY_LEN];
	snprintf(key, sizeof(key), "%020llu", seq);

	size_t  len = 0;
	void    *value = NULL;

	int res = __load_string_bin(&ctx->kv_ops, db->db_handler, key, &value, &len);
	assert(res != KV_LOAD_FAIL);

	if (res == KV_LOAD_NULL) {
		return NULL;
	}

	return (xmq_msg_t *)value;
}

int xmq_update_that(xmq_consumer_t *consumer, uint64_t seq)
{
	assert(consumer);

	xmq_ctx_t *ctx = consumer->xmq_ctx;

	/* Update XMQ_CN_FETCHPOS */
	int res = __write_uint64(&ctx->kv_ops, ctx->g_state_ctx, consumer->fetch_key, seq);
	return_if_false((res == 0), -1);

	consumer->last_fetch_seq = seq;
	return 0;
}

uint64_t xmq_fetch_position(xmq_consumer_t *consumer)
{
	return consumer->last_fetch_seq;
}

int __load_all_resouces(xmq_ctx_t *ctx)
{
	return_if_false(ctx, -1);

	/* Initialize K/V state context,
	 *   it stores all the global database/producer/consumer 's status.
	 * */
	if (!(ctx->g_state_ctx = ctx->kv_ops.kv_ctx_init("./kvstate"))) {
		x_printf(W, "Initialize the global state K/V database './kvstate' fail.");
		return -1;
	}

	/* First loading the XMQ_DATABASE_WRITEPOS (0~18446744073709551615UL)*/
	uint64_t        write_pos = 0;
	int             res = __load_uint64(&ctx->kv_ops, ctx->g_state_ctx, XMQ_DATABASE_WRITEPOS, &write_pos);

	if (res == KV_LOAD_FAIL) {
		x_printf(F, "__load_uint64(XMQ_DATABASE_WRITEPOS) fail. K/V database error.");
		return -1;
	} else if (res == KV_LOAD_NULL) {
		x_printf(I, "Loading the XMQ_DATABASE_WRITEPOS fail. we'll create it for the first time.");
		write_pos = 1;
		res = __write_uint64(&ctx->kv_ops, ctx->g_state_ctx, XMQ_DATABASE_WRITEPOS, write_pos);
		return_if_false((res == 0), -1);
	} else {
		x_printf(D, "__load_uint64(XMQ_DATABASE_WRITEPOS) %llu.", write_pos);
	}

	/* Loading all the used databases to list_databases.
	 *  [like: "00000000000000000001-00000000000005000000"] to list_databases.
	 * All the used database will be opend for consumers reading.
	 */
	char name[XMQ_KEY_LEN];
	memset(name, 0, sizeof(name));
	snprintf(name, sizeof(name), "%llu", write_pos);
	x_printf(D, "__load_uint64->xmq_databases=[%s].", name);

	int databases = __load_string_list(ctx, name, &ctx->list_databases, XMQ_TYPE_DATABASE);

	if (databases > 0) {
		x_printf(I, "Loding %d of databases to list_databases succeed.", databases);
	} else {
		x_printf(F, "Loding databases to list_databases fail. process return -1.");
		return -1;
	}

	ctx->last_write_db_seq = write_pos;
	ctx->last_write_db = __get_db_handler(ctx, write_pos);
	assert(ctx->last_write_db);

	/* Loding all the producers to list. */
	int producers = __load_string_list(ctx, XMQ_PRODUCERS, &ctx->list_producers, XMQ_TYPE_PRODUCER);

	if (producers > 0) {
		x_printf(I, "Loding %d of producers to list_producers succeed.", producers);
	} else if (producers == 0) {
		x_printf(I, "Loding producers to list_producers fail. There's no producer in database. It will be registered in time.");
	} else {
		x_printf(F, "Loding producers to list_producers fail. process return -1.");
		return -1;
	}

	/* Loding all the consumers to list. */
	int consumers = __load_string_list(ctx, XMQ_CONSUMERS, &ctx->list_consumers, XMQ_TYPE_CONSUMER);

	if (consumers > 0) {
		x_printf(I, "Loding %d of consumers to list_consumers succeed.", consumers);
	} else if (consumers == 0) {
		x_printf(I, "Loding consumers to list_consumers fail. There's no consumer registered or reading the K/V database.");
	} else {
		x_printf(F, "Loding consumers to list_consumers fail. process return -1.");
		return -1;
	}

	return 0;
}

xmq_db_t *__get_db_handler(xmq_ctx_t *ctx, uint64_t fetchpos)
{
	return_if_false(ctx, NULL);

	xlist_t *iter;
	list_foreach_reverse(iter, &ctx->list_databases)
	{
		xmq_db_t *db = container_of(iter, xmq_db_t, node);

		/* e.g. [1,5000],[5001,10000] */
		if ((db->db_start <= fetchpos) && (fetchpos <= db->db_end)) {
			// x_printf(D, "Found the database [%s], fetchpos:%llu", db->db_name, fetchpos);
			return db;
		}
	}

	return NULL;
}

void *__is_obj_exist(xmq_ctx_t *ctx, const char *key, int type)
{
	return_if_false((ctx && key), (char *)-1);

	xlist_t *iter = NULL;

	switch (type)
	{
		case XMQ_TYPE_DATABASE:
		{
			list_foreach(iter, &ctx->list_databases)
			{
				xmq_db_t *db = container_of(iter, xmq_db_t, node);

				if (!(strncmp(db->db_name, key, strlen(key) + 1))) {
					x_printf(D, "Database [%s] was found.", key);
					return db;
				}
			}
			break;
		}

		case XMQ_TYPE_PRODUCER:
		{
			list_foreach(iter, &ctx->list_producers)
			{
				xmq_producer_t *producer = container_of(iter, xmq_producer_t, node);

				if (!(strncmp(producer->identity, key, strlen(key) + 1))) {
					x_printf(D, "Producer [%s] was found.", key);
					return producer;
				}
			}
			break;
		}

		case XMQ_TYPE_CONSUMER:
		{
			list_foreach(iter, &ctx->list_consumers)
			{
				xmq_consumer_t *consumer = container_of(iter, xmq_consumer_t, node);

				if (!(strncmp(consumer->identity, key, strlen(key) + 1))) {
					x_printf(D, "Consumer [%s] was found.", key);
					return consumer;
				}
			}
			break;
		}

		default:
			x_printf(W, "__is_obj_exist: type is invalid.");
	}
	return NULL;
}

int __open_database(xmq_ctx_t *ctx, uint64_t last_writepos)
{
	return_if_false(ctx, -1);

	/* last_writepos's value % ctx->kv_max_records == 1 */	// TODO:add
	if (!__get_db_handler(ctx, last_writepos)) {
		char dbname[XMQ_DBNAME_LEN];
		__get_KV_dbname(dbname, last_writepos, ctx->kv_max_records - 1);

		if (__load_database(ctx, dbname)) {
			x_printf(W, "__load_database('%s') new database fail.", dbname);
			return -1;
		}

		ctx->last_write_db = __get_db_handler(ctx, last_writepos);
		x_printf(I, "__get_db_handler() new database [%s] succeed.", ctx->last_write_db->db_name);
	}

	return 0;
}

int __load_database(xmq_ctx_t *ctx, const char *name)
{
	return_if_false((ctx && name), -1);

	int     space = strlen(ctx->kv_db_path) + strlen(name) + 2;
	char    dbpath[space];
	snprintf(dbpath, space, "%s/%s", ctx->kv_db_path, name);

	void *kv_ctx = ctx->kv_ops.kv_ctx_init(dbpath);

	if (!kv_ctx) {
		x_printf(F, "Create/Open the database [%s] fail.", dbpath);
		return -1;
	}

	xmq_db_t *db = (xmq_db_t *)calloc(1, sizeof(xmq_db_t));
	return_if_false(db, -1);

	/* name like: 00000000000000000001-00000000000000500000 */
	strncpy(db->db_name, name, sizeof(db->db_name));
	db->db_start = strtoull(name, NULL, 10);
	db->db_end = strtoull(name + XMQ_KEY_LEN, NULL, 10);
	db->db_handler = kv_ctx;

	list_init(&db->node);
	list_add_tail(&db->node, &ctx->list_databases);
	x_printf(I, "__load_database{name:'%s', start:%llu end:%llu, db_handler:0x%lx} succeed.", name, db->db_start, db->db_end, db->db_handler);

	return 0;
}

int __unload_database(xmq_ctx_t *ctx, const char *name)
{
	return_if_false((ctx && name), -1);

	xmq_db_t *db = (xmq_db_t *)__is_obj_exist(ctx, name, XMQ_TYPE_DATABASE);

	if (!db) {
		x_printf(W, "__unload_database() fail. database ('%s') does not exist.", name);
		return -1;
	}

	list_del(&db->node);
	ctx->kv_ops.kv_ctx_free(db->db_handler);
	free(db);
	x_printf(I, "__unload_database(%s) succeed.", name);

	return 0;
}

int __list_to_csv(xlist_t *head, csv_parse_t *csv, int type)
{
	return_if_false((head && csv), -1);
}

int __csv_to_list(xmq_ctx_t *ctx, csv_parse_t *csv, xlist_t *head, int type)
{
	return_if_false((ctx && csv && head), -1);

	csv_field_t *field;

	switch (type)
	{
		case XMQ_TYPE_DATABASE:
			for_each_field(field, csv)
			{
				char dbname[XMQ_DBNAME_LEN] = { 0 };

				strncpy(dbname, field->ptr, field->len);

				if (__load_database(ctx, dbname)) {
					x_printf(W, "Loding database [%s] to list_database fail, returned.", dbname);
					return -1;
				}

				x_printf(I, "Loding database [%s] to list_database succeed.", dbname);
			}

			break;

		case XMQ_TYPE_PRODUCER:
			for_each_field(field, csv)
			{
				xmq_producer_t *producer = (xmq_producer_t *)calloc(1, sizeof(xmq_producer_t));

				return_if_false(producer, -1);

				strncpy(producer->identity, field->ptr, field->len);
				producer->xmq_ctx = ctx;

				list_init(&producer->node);
				list_add_tail(&producer->node, &ctx->list_producers);
				x_printf(I, "Loding producer [%s] to list_producers succeed.", producer->identity);
			}

			break;

		case XMQ_TYPE_CONSUMER:
			for_each_field(field, csv)
			{
				xmq_consumer_t *consumer = (xmq_consumer_t *)calloc(1, sizeof(xmq_consumer_t));

				return_if_false(consumer, -1);

				strncpy(consumer->identity, field->ptr, field->len);
				consumer->xmq_ctx = ctx;

				/* Loading the last_fetchpos. */
				char *key_fetchpos = key_of_consumer_fetchpos(consumer->identity);
				strcpy(consumer->fetch_key, key_fetchpos);
				free(key_fetchpos);

				uint64_t        fetchpos = 0;
				int             res = __load_uint64(&ctx->kv_ops, ctx->g_state_ctx, consumer->fetch_key, &fetchpos);
				return_if_false((res == KV_LOAD_SUCC), -1);
				consumer->last_fetch_seq = fetchpos;

				list_init(&consumer->node);
				list_add_tail(&consumer->node, &ctx->list_consumers);
				x_printf(I, "Loding consumer [%s] last_fetchpos:%llu to list_consumers succeed.", consumer->identity, consumer->last_fetch_seq);
			}

			break;

		default:
			x_printf(E, "__laod_string_list, Invalid XMQ_TYPE_XX.");
			return -1;
	}
	return 0;
}

int __load_string_list(xmq_ctx_t *ctx, const char *key, xlist_t *head, enum xmq_type type)
{
	return_if_false((ctx && key && head), -1);

	char *value = NULL;

	switch (type)
	{
		case XMQ_TYPE_DATABASE:
			/* e.g. key = 2847202334 */
		{
			uint64_t last_write_pos = strtoull(key, NULL, 10);

			size_t  dbs = GET_NEED_COUNT(last_write_pos, ctx->kv_max_records);
			size_t  len = dbs * XMQ_DBNAME_LEN + 1;
			value = (char *)calloc(1, len);

			uint64_t i = 0;

			for (i = 1; i <= last_write_pos; i += ctx->kv_max_records) {
				char dbname[XMQ_DBNAME_LEN];
				__get_KV_dbname(dbname, i, ctx->kv_max_records - 1);

				/* 00000000000000000001-00000000000005000000,00000000000005000001-00000000000010000000, */
				strcat(value, dbname); strcat(value, ",");
			}

			value[len - 2] = '\0';
			x_printf(D, "All the databases is:\n\t%s\n", value);
		}
		break;

		default:
			// Others, such as: XMQ_TYPE_PRODUCER|XMQ_TYPE_CONSUMER
		{
			size_t  len;
			int     res = __load_string_bin(&ctx->kv_ops, ctx->g_state_ctx, key, (void **)&value, &len);
			return_if_false((res != KV_LOAD_FAIL), -1);

			if (res == KV_LOAD_NULL) {
				x_printf(I, "__load_string_string: (Key:%s Val:(char *)1). The Key doesn't exist.", key);
				return 0;
			}
		}
		break;
	}

	x_printf(D, "__load_string_string: KEY:<%s> VALUE:<%s>", key, value);

	csv_parse_t csv;
	xmq_csv_parse_init(&csv);
	int fields = xmq_csv_parse_string(&csv, value);
	free(value);
	return_if_false((fields != -1), -1);

	if (-1 == __csv_to_list(ctx, &csv, head, type)) {
		x_printf(E, "__csv_to_list: (type:%s) failed!",
			(type == XMQ_TYPE_DATABASE) ? "XMQ_TYPE_DATABASE" :
			((type == XMQ_TYPE_PRODUCER) ? "XMQ_TYPE_PRODUCER" : "XMQ_TYPE_CONSUMER"));

		xmq_csv_parse_destroy(&csv);
		return 0;
	}

	xmq_csv_parse_destroy(&csv);

	return fields;
}

int __write_back_append(xmq_ctx_t *ctx, const char *key, const char *append, const char *split)
{
	return_if_false((ctx && key && append && split), -1);

	size_t  len;
	char    *value_old = NULL;
	int     res = __load_string_bin(&ctx->kv_ops, ctx->g_state_ctx, key, (void **)&value_old, &len);
	return_if_false((res != KV_LOAD_FAIL), -1);

	char *value_new = __string_append_3rd(value_old, split, append, NULL, NULL);
	freeif(value_old);

	if (!value_new) {
		x_printf(W, "__string_append_3rd: [split:%s,append:%s] fail.", split, append);
		return -1;
	}

	int rc = __write_string_string(ctx, key, value_new);
	free(value_new);

	if (rc == -1) {
		x_printf(W, "__write_string_string: [Key:%s] fail.", key);
		return -1;
	}

	return rc;
}

int __write_back_delete(xmq_ctx_t *ctx, const char *key, const char *delete, const char *split)
{
	return_if_false((ctx && key && delete && split), -1);

	size_t  len;
	char    *value_old = NULL;

	int res = __load_string_bin(&ctx->kv_ops, ctx->g_state_ctx, key, (void **)&value_old, &len);
	return_if_false((value_old == KV_LOAD_SUCC), -1);

	if (!__string_delete_by_substr(value_old, delete, split)) {
		x_printf(W, "__string_delete_by_substr(src:%s, del:%s) error found.", value_old, delete);
		free(value_old);
		return -1;
	}

	if (__write_string_string(ctx, key, value_old)) {
		x_printf(W, "__write_string_string(,key:%s, val:%s,) fail.", key, value_old);
		free(value_old);
		return -1;
	}

	free(value_old);

	return 0;
}

int __write_int(xmq_ops_t *ops, void *kv_ctx, const char *key, int value)
{
	return __write_string_bin(ops, kv_ctx, key, (void *)&value, sizeof(int));
}

int __write_uint64(xmq_ops_t *ops, void *kv_ctx, const char *key, uint64_t value)
{
	return __write_string_bin(ops, kv_ctx, key, (void *)&value, sizeof(uint64_t));
}

int __write_string_bin(xmq_ops_t *ops, void *kv_ctx, const char *key, const void *value, size_t len)
{
	return_if_false((ops && kv_ctx && key && value && len), -1);

	bin_entry_t bin_k, bin_v;
	set_bin_entry(&bin_k, key, strlen(key) + 1);
	set_bin_entry(&bin_v, value, len);

	return ops->kv_put(kv_ctx, &bin_k, &bin_v, 1);
}

int __write_string_string(xmq_ctx_t *ctx, const char *key, const char *value)
{
	return __write_string_bin(&ctx->kv_ops, ctx->g_state_ctx, key, value, strlen(value) + 1);
}

int __load_int(xmq_ops_t *ops, void *kv_ctx, const char *key, int *value)
{
	size_t  len = 0;
	int     *val = NULL;

	int res = __load_string_bin(ops, kv_ctx, key, (void **)&val, &len);

	return_if_false((res == 0), res);

	*value = *val;
	free(val);

	return 0;
}

int __load_uint64(xmq_ops_t *ops, void *kv_ctx, const char *key, uint64_t *value)
{
	size_t          len = 0;
	uint64_t        *u64 = NULL;

	int res = __load_string_bin(ops, kv_ctx, key, (void **)&u64, &len);

	return_if_false((res == KV_LOAD_SUCC), res);

	assert(len == sizeof(uint64_t));
	*value = *u64;
	free(u64);

	return res;
}

/* Do not found the value by key, return 1.
 * LevelDB option error return -1.Succed return 0.*/
int __load_string_bin(xmq_ops_t *ops, void *kv_ctx, const char *key, void **value, size_t *len)
{
	return_if_false((ops && kv_ctx && key && value && len), -1);

	bin_entry_t b_key, b_val;
	set_bin_entry(&b_key, key, strlen(key) + 1);

	int res = ops->kv_get(kv_ctx, &b_key, &b_val, 1);

	if (res == -1) {
		return KV_LOAD_FAIL;
	}

	if (b_val.data == NULL) {
		x_printf(W, "ops->kv_get(KEY:%s, VAL: null). There isn't this key.", key);
		*value = NULL;
		*len = 0;
		return KV_LOAD_NULL;
	} else {
		*value = b_val.data;
		*len = b_val.len;
		return KV_LOAD_SUCC;
	}
}

char *__string_delete_by_substr(char *src, const char *substr, const char *split)
{
	return_if_false(src && substr, NULL);

#if 0
	/* Situation: src="master" substr="master" */
	if (strlen(src) == strlen(substr)) {
		return (strcmp(src, substr)) ? src : (src[0] = '\0', src);
	}
#endif
	int     len = strlen(substr);
	char    *pos = src;
	do {
		pos = strstr(pos, substr);
		return_if_false(pos, src);

		if (strcmp(pos + len, split) && (pos[len] != '\0')) {
			pos += len;
		} else {
			strcpy(pos, pos + len);
		}
	} while (1);
}

char *__string_append_3rd(const char *prefix, const char *split, const char *first, const char *second, const char *third)
{
	return_if_false(split, NULL);

	size_t total = 1;

	if (prefix) {
		total += strlen(prefix);
	}

	if (first) {
		total += strlen(first) + strlen(split);
	}

	if (second) {
		total += strlen(second) + strlen(split);
	}

	if (third) {
		total += strlen(third) + strlen(split);
	}

	char *merge = (char *)calloc(1, total);

	if (merge) {
		if (prefix) {
			strcat(merge, prefix);
		}

		if (first) {
			if (prefix) {
				strcat(merge, split);
			}

			strcat(merge, first);

			if (second) {
				strcat(merge, split);
				strcat(merge, second);

				if (third) {
					strcat(merge, split);
					strcat(merge, third);
				}
			}
		}
	}

	x_printf(D, "__string_append_3rd: return [%s]", merge);
	return merge;
}

void __get_KV_dbname(char dest[XMQ_DBNAME_LEN], uint64_t start, size_t offset)
{
	char temp[2][XMQ_KEY_LEN];

	snprintf(temp[0], sizeof(temp[0]), "%020llu", start);
	snprintf(temp[1], sizeof(temp[1]), "%020llu", start + offset);
	snprintf(dest, XMQ_DBNAME_LEN, "%s-%s", temp[0], temp[1]);
}

