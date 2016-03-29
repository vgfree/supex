#include "xmq.h"
#include "xmq_csv.h"
#include "slog/slog.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
 * XMQ_KEY_LEN 21  "18446744073709551615"  __u64toa()/__atou64() to change.
 * */
#define XMQ_KEY_LEN             21	// "18446744073709551615" len=20, 1 for '\0'
#define XMQ_U64_MAX             (18446744073709551615UL)

#define XMQ_TYPE_DATABASE       1
#define XMQ_TYPE_PRODUCER       2
#define XMQ_TYPE_CONSUMER       3

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

/* __atou64, string type with "00000000000002378923" convert to uint64_t,
 * len = strlen(src) or the length you want to convert, from src. */
static uint64_t __atou64(const char *str, size_t len);

/* __u64toa, uint64_t type change to string "00000000000002378923" */
static char *__u64toa(uint64_t u64, char *des, int len);

/* __get_KV_dbname() get K/V database name, [1,5000],[5001,10000] -> offset = 4999 */
static char *__get_KV_dbname(uint64_t start, size_t offset);

/* string append with [first,second,third] by split. */
static char *__string_append_3rd(const char *prefix, const char *split, const char *first, const char *second, const char *third);

/* Delete the substr, and return the left string. */
static char *__string_delete_by_substr(char *src, const char *substr);

/* Append string 'append' and rewrite to database. */
static int __write_back_append(xmq_ctx_t *ctx, const char *key, const char *append, const char *split);

/* Delete string 'del' and rewrite to database. */
static int __write_back_delete(xmq_ctx_t *ctx, const char *key, const char *del);

static int __load_int(xmq_ctx_t *ctx, void *kv_ctx, const char *key, int *value);

static int __write_int(xmq_ctx_t *ctx, void *kv_ctx, const char *key, int value);

static int __load_uint64(xmq_ctx_t *ctx, void *kv_ctx, const char *key, uint64_t *value);

static int __write_uint64(xmq_ctx_t *ctx, void *kv_ctx, const char *key, uint64_t value);

/* The main function to operator the K/V database. */
static int __load_string_bin(xmq_ctx_t *ctx, void *kv_ctx, const char *key, void **value, size_t *len);

static int __write_string_bin(xmq_ctx_t *ctx, void *kv_ctx, const char *key, const void *value, size_t len);

/* Only used for read or write ./kvstate DB. */
static char *__load_string_string(xmq_ctx_t *ctx, const char *key);

static int __write_string_string(xmq_ctx_t *ctx, const char *key, const char *value);

/* Loading such as "P1,P2,P3,..PN to list_XXX." */
static int __load_string_list(xmq_ctx_t *ctx, const char *key, xlist_t *head, int type);

static int __list_to_csv(xlist_t *head, csv_parser_t *csv, int type);

static int __csv_to_list(xmq_ctx_t *ctx, csv_parser_t *csv, xlist_t *head, int type);

/* Only open the K/V database and load it to list_database or delete from it.*/
static int __load_database(xmq_ctx_t *ctx, const char *name);

static int __unload_database(xmq_ctx_t *ctx, const char *name);

static int __open_database(xmq_ctx_t *ctx, uint64_t last_writepos);

/* Loading all the resources was called by xmq_context_init. */
static int __load_all_resouces(xmq_ctx_t *ctx);

static int __mkdir_if_doesnt_exist(const char *dirpath);

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
	xmq_kv_ctx_destroy *kv_ctx_destroy)
{
	return_if_false((kv_db_path && kv_max_records > 0 && kv_ctx_init && kv_put && kv_get && kv_ctx_destroy), NULL);

	/* You should never change calloc to malloc. calloc will clear memory with '\0'*/
	xmq_ctx_t *ctx = (xmq_ctx_t *)calloc(1, sizeof(xmq_ctx_t));
	return_if_false(ctx, NULL);

	strncpy(ctx->kv_db_path, kv_db_path, XMQ_PATH_LEN - 1);
	ctx->kv_max_records = kv_max_records;
	ctx->kv_ctx_init = kv_ctx_init;
	ctx->kv_put = kv_put;
	ctx->kv_get = kv_get;
	ctx->kv_ctx_destroy = kv_ctx_destroy;

	if (__mkdir_if_doesnt_exist(kv_db_path)) {
		x_printf(E, "__mkdir_if_doesnt_exist: Execute fail.");
		free(ctx); return NULL;
	}

	list_init(&ctx->list_databases);
	list_init(&ctx->list_consumers);
	list_init(&ctx->list_producers);

	pthread_mutex_init(&ctx->lock_write, NULL);
	pthread_cond_init(&ctx->lock_cond, NULL);
	ctx->waiters = 0;

	if (__load_all_resouces(ctx)) {
		x_printf(W, "__load_all_resouces fail. Initialize fail.");
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
			ctx->kv_ctx_destroy(db->db_handler);
			free(db);
		}
		ctx->kv_ctx_destroy(ctx->g_state_ctx);

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

int xmq_register_consumer(xmq_ctx_t *ctx, const char *cid)
{
	return_if_false((ctx && cid), -1);

	/* Checksum, if it doesn't exist, then create it and load it to list_consumers.*/
	if (!__is_obj_exist(ctx, cid, XMQ_TYPE_CONSUMER)) {
		x_printf(I, "consumer [%s] doesn't exist. we'll register it.", cid);

		/* Create consumer read state to db './kvstate' and add to list_consumers */
		xmq_consumer_t *consumer = (xmq_consumer_t *)calloc(1, sizeof(xmq_consumer_t));
		return_if_false(consumer, -1);

		consumer->xmq_ctx = ctx;
		consumer->last_fetch_seq = 1;
		strncpy(consumer->identity, cid, sizeof(consumer->identity) - 1);

		char *key_fetchpos = key_of_consumer_fetchpos(consumer->identity);
		strcpy(consumer->fetch_key, key_fetchpos);
		free(key_fetchpos);

		// Append this consumer to XMQ_CONSUMERS in db kvstate, and set relevant macro fields. see: key_of_XXX.
		if (__write_back_append(ctx, XMQ_CONSUMERS, consumer->identity, ",")) {
			x_printf(W, "__write_back_append(Key:%s Value:[%s]) fail", XMQ_CONSUMERS, consumer->identity);
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
	return (xmq->indicator->next != &xmq->list_consumers);
}

static xmq_consumer_t *next(xmq_ctx_t *xmq)
{
	xmq->indicator = xmq->indicator->next;

	return (xmq_consumer_t *)container_of(xmq->indicator, xmq_consumer_t, node);
}

xmq_consiter_t xmq_get_consumer_iterator(xmq_ctx_t *xmq)
{
	xmq->indicator = &xmq->list_consumers;

	xmq_consiter_t  xiter = {.next = next, .has_next = has_next };

	return xiter;
}

int xmq_unregister_producer(xmq_ctx_t *ctx, const char *pid)
{
	return_if_false((ctx && pid), -1);

	xmq_producer_t *producer = (xmq_producer_t *)__is_obj_exist(ctx, pid, XMQ_TYPE_PRODUCER);

	if (producer) {
		list_del(&producer->node);

		/* Reset the state of this producer in db kvstate. */
		if (__write_back_delete(ctx, XMQ_PRODUCERS, producer->identity)) {
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
		if (__write_back_delete(ctx, XMQ_CONSUMERS, consumer->identity)) {
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

	/* 1. Checking: uint64_t is full with sequence = XMQ_U64_MAX */
	if (ctx->last_write_db_seq + 1 == XMQ_U64_MAX) {
		x_printf(W, "The last write sequence is 18446744073709551615, will stop push any message.");
		return -1;
	}

	pthread_mutex_lock(&ctx->lock_write);

	/* 2. Checking: One DB was write full!
	 * Create a new database to store K/V data, load it to list_database. */
	if ((ctx->last_write_db_seq % ctx->kv_max_records) == 1) {
		if (__open_database(ctx, ctx->last_write_db_seq)) {
			pthread_mutex_unlock(&ctx->lock_write);
			x_printf(W, "__open_database(last_writepos:%llu) fail.", ctx->last_write_db_seq);
			return -1;
		}
	}

	/* Write new K/V data to database [0~N]. */
	char key[XMQ_KEY_LEN];
	__u64toa(ctx->last_write_db_seq, key, sizeof(key));

	if (__write_string_bin(ctx, ctx->last_write_db->db_handler, key, msg, xmq_msg_total_size(msg))) {
		pthread_mutex_unlock(&ctx->lock_write);
		x_printf(F, "__write_string_bin(key:%s, value:%s) fail.", key, (char *)msg->data);
		return -1;
	}

	/* Update XMQ_DATABASE_WRITEPOS */
	if (__write_uint64(ctx, ctx->g_state_ctx, XMQ_DATABASE_WRITEPOS, ctx->last_write_db_seq)) {
		pthread_mutex_unlock(&ctx->lock_write);
		x_printf(F, "__write_uint64(key:%s, value:%lld) fail.", XMQ_DATABASE_WRITEPOS, ctx->last_write_db_seq);
		return -1;
	}

	ctx->last_write_db_seq++;

	// If waiters (for fetching the XMQ queue) is non-zero, we wake up them.
	if (ctx->waiters) {
		pthread_cond_broadcast(&ctx->lock_cond);
	}

	pthread_mutex_unlock(&ctx->lock_write);

	return 0;
}

xmq_msg_t *xmq_fetch_nth(xmq_consumer_t *consumer, uint64_t seq, long timeout)
{
	return_if_false((consumer && timeout >= -1), (xmq_msg_t *)-1);

	/* 1. Checking: uint64_t is full with sequence = XMQ_U64_MAX */
	if (seq == XMQ_U64_MAX) {
		x_printf(W, "Sequence of you request is 18446744073709551615 or (uint64_t)-1, we'll stop fetch next message.");
		return NULL;
	}

	xmq_ctx_t *ctx = consumer->xmq_ctx;

	xmq_db_t *db = (xmq_db_t *)__get_db_handler(ctx, seq);

	if (db == NULL) {
		x_printf(F, "__get_db_handler(fetchpos:%llu) fail, database doesn't exist.", consumer->last_fetch_seq + 1);
		return NULL;
	}

	char key[XMQ_KEY_LEN];
	__u64toa(seq, key, sizeof(key));

	int     res = -1;
	size_t  len = 0;
	void    *value = NULL;

	// __load_string_bin only return 1 or 0.
	//   1(no data returned) or 0(with data returned).
	if (__load_string_bin(ctx, db->db_handler, key, &value, &len)) {
		pthread_mutex_lock(&ctx->lock_write);

		switch (timeout)
		{
			case  0:
				pthread_mutex_unlock(&ctx->lock_write);
				return NULL;

			case -1:

				while (value == NULL) {
					++(ctx->waiters);
					res = pthread_cond_wait(&ctx->lock_cond, &ctx->lock_write);
					--(ctx->waiters);

					__load_string_bin(ctx, db->db_handler, key, &value, &len);

					if (res != 0) {
						pthread_mutex_unlock(&ctx->lock_write);
						x_printf(E, "pthread_cond_wait Execute fail. Error-%s.", strerror(res));
						return NULL;
					}
				}

				break;

			default:

				while (value == NULL) {
					struct timespec ts;
					clock_gettime(CLOCK_REALTIME, &ts);
					ts.tv_nsec += (timeout * 1000000);
					res = ts.tv_nsec / 1000000000;

					if (res > 0) {
						ts.tv_nsec %= 1000000000;
						ts.tv_sec += res;
					}

					++(ctx->waiters);
					res = pthread_cond_timedwait(&ctx->lock_cond, &ctx->lock_write, &ts);
					--(ctx->waiters);

					__load_string_bin(ctx, db->db_handler, key, &value, &len);

					if (res != 0) {
						pthread_mutex_unlock(&ctx->lock_write);

						if (res != ETIMEDOUT) {
							x_printf(E, "pthread_cond_timedwait Execute fail. Error-%s.", strerror(res));
						}

						return NULL;
					}
				}
		}

		pthread_mutex_unlock(&ctx->lock_write);
	}

	/* Update XMQ_CN_FETCHPOS */
	res = __write_uint64(ctx, ctx->g_state_ctx, consumer->fetch_key, seq);
	return_if_false((res == 0), NULL);

	consumer->last_fetch_seq = seq;

	return (xmq_msg_t *)value;
}

xmq_msg_t *xmq_fetch_next(xmq_consumer_t *consumer)
{
	return xmq_fetch_nth(consumer, consumer->last_fetch_seq + 1, -1);
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
	if (!(ctx->g_state_ctx = ctx->kv_ctx_init("./kvstate"))) {
		x_printf(W, "Initialize the global state K/V database './kvstate' fail.");
		return -1;
	}

	/* First loading the XMQ_DATABASE_WRITEPOS (0~18446744073709551615UL)*/
	uint64_t        write_pos = 0;
	int             res = __load_uint64(ctx, ctx->g_state_ctx, XMQ_DATABASE_WRITEPOS, &write_pos);

	if (res == 0) {
		/* Loading all the used databases to list_databases.
		 *  [like: "00000000000000000001-00000000000005000000"] to list_databases.
		 * All the used database will be opend for consumers reading.
		 */
		char s_xmq_databases[14 + XMQ_KEY_LEN];	// 14 is length of "XMQ_DATABASES"
		sprintf(s_xmq_databases, "XMQ_DATABASES%llu", write_pos);
		x_printf(D, "__load_uint64->s_xmq_databases=[%s].", s_xmq_databases);

		int dbs = __load_string_list(ctx, s_xmq_databases, &ctx->list_databases, XMQ_TYPE_DATABASE);

		if (dbs > 0) {
			x_printf(I, "Loding %d of databases to list_databases succeed.", dbs);
		} else {
			x_printf(F, "Loding databases to list_databases fail. process return -1.");
			return -1;
		}

		ctx->last_write_db_seq = write_pos;
		ctx->last_write_db = __get_db_handler(ctx, write_pos);

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
	} else if (res == 1) {
		x_printf(I, "Loading the XMQ_DATABASE_WRITEPOS fail. we'll create it for the first time.");

		int res = __write_uint64(ctx, ctx->g_state_ctx, XMQ_DATABASE_WRITEPOS, 1);
		return_if_false((res == 0), -1);

		ctx->last_write_db_seq = 1;

		if (__open_database(ctx, ctx->last_write_db_seq)) {
			x_printf(W, "__open_database(last_writepos:%llu) for the first time fail.", ctx->last_write_db_seq);
			return -1;
		}
	} else {
		x_printf(F, "__load_uint64(XMQ_DATABASE_WRITEPOS) fail. K/V database error.");
		return -1;
	}

	return 0;
}

xmq_db_t *__get_db_handler(xmq_ctx_t *ctx, uint64_t fetchpos)
{
	return_if_false(ctx, (xmq_db_t *)-1);

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
					// x_printf(D, "Database [%s] was found.", key);
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
					// x_printf(D, "Producer [%s] was found.", key);
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
					// x_printf(D, "Consumer [%s] was found.", key);
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

	/* last_writepos's value % ctx->kv_max_records == 1 */
	if (!__get_db_handler(ctx, last_writepos)) {
		char *dbname = __get_KV_dbname(last_writepos, ctx->kv_max_records - 1);

		if (__load_database(ctx, dbname)) {
			x_printf(W, "__load_database('%s') new database fail.", dbname);
			free(dbname); return -1;
		}

		free(dbname);

		ctx->last_write_db = __get_db_handler(ctx, last_writepos);
		x_printf(I, "__get_db_handler() new database [%s] succeed.", ctx->last_write_db->db_name);
	}

	return 0;
}

int __load_database(xmq_ctx_t *ctx, const char *name)
{
	return_if_false((ctx && name), -1);

	char *dbpath = (char *)malloc(strlen(ctx->kv_db_path) + strlen(name) + 2);
	return_if_false(dbpath, -1);

	sprintf(dbpath, "%s/%s", ctx->kv_db_path, name);

	void *kv_ctx = ctx->kv_ctx_init(dbpath);

	if (!kv_ctx) {
		x_printf(F, "Create/Open the database [%s] fail.", dbpath);
		free(dbpath); return -1;
	}

	free(dbpath);

	xmq_db_t *db = (xmq_db_t *)calloc(1, sizeof(xmq_db_t));
	return_if_false(db, -1);

	/* name like: 00000000000000000001-00000000000000500000 */
	strncpy(db->db_name, name, sizeof(db->db_name) - 1);
	db->db_start = __atou64(name, XMQ_KEY_LEN - 1);
	db->db_end = __atou64(name + XMQ_KEY_LEN, XMQ_KEY_LEN - 1);
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
	ctx->kv_ctx_destroy(db->db_handler);
	free(db);
	x_printf(I, "__unload_database(%s) succeed.", name);

	return 0;
}

int __list_to_csv(xlist_t *head, csv_parser_t *csv, int type)
{
	return_if_false((head && csv), -1);
}

int __csv_to_list(xmq_ctx_t *ctx, csv_parser_t *csv, xlist_t *head, int type)
{
	return_if_false((ctx && csv && head), -1);

	csv_field_t *field;

	for_each_field(field, csv)
	{
		switch (type)
		{
			case XMQ_TYPE_DATABASE:
			{
				char db[XMQ_DBNAME_LEN] = { 0 };
				strncpy(db, field->ptr, field->len);

				if (__load_database(ctx, db)) {
					x_printf(W, "Loding database [%s] to list_database fail, returned.", db);
					return -1;
				}

				x_printf(I, "Loding database [%s] to list_database succeed.", db);

				break;
			}

			case XMQ_TYPE_PRODUCER:
			{
				xmq_producer_t *producer = (xmq_producer_t *)calloc(1, sizeof(xmq_producer_t));
				return_if_false(producer, -1);

				strncpy(producer->identity, field->ptr, field->len);
				producer->xmq_ctx = ctx;

				list_init(&producer->node);
				list_add_tail(&producer->node, &ctx->list_producers);
				x_printf(I, "Loding producer [%s] to list_producers succeed.", producer->identity);

				break;
			}

			case XMQ_TYPE_CONSUMER:
			{
				xmq_consumer_t *consumer = (xmq_consumer_t *)calloc(1, sizeof(xmq_consumer_t));
				return_if_false(consumer, -1);

				strncpy(consumer->identity, field->ptr, field->len);

				/* Loading the last_fetchpos. */
				uint64_t        fetchpos;
				char            *key_fetchpos = key_of_consumer_fetchpos(consumer->identity);
				strcpy(consumer->fetch_key, key_fetchpos);

				int res = __load_uint64(ctx, ctx->g_state_ctx, key_fetchpos, &fetchpos);
				free(key_fetchpos);
				return_if_false((res == 0), -1);

				consumer->last_fetch_seq = fetchpos;
				consumer->xmq_ctx = ctx;

				list_init(&consumer->node);
				list_add_tail(&consumer->node, &ctx->list_consumers);
				x_printf(I, "Loding consumer [%s] last_fetchpos:%llu to list_consumers succeed.", consumer->identity, consumer->last_fetch_seq);

				break;
			}

			default:
			{
				x_printf(E, "__laod_string_list, Invalid XMQ_TYPE_XX.");
				return -1;
			}
		}
	}
	return 0;
}

int __load_string_list(xmq_ctx_t *ctx, const char *key, xlist_t *head, int type)
{
	return_if_false((ctx && key && head), -1);

	char *value = NULL;

	/* e.g. key = XMQ_DATABASES2847202334 */
	if (!strncmp(key, "XMQ_DATABASES", strlen(XMQ_DATABASES))) {
		uint64_t        i, last_write_pos = 0;
		const char      *ptr = key + strlen(XMQ_DATABASES);
		last_write_pos = __atou64(ptr, strlen(ptr));

		size_t dbs = (last_write_pos / ctx->kv_max_records) + ((last_write_pos % ctx->kv_max_records) ? 1 : 0);
		value = (char *)calloc(1, dbs * (2 * XMQ_DBNAME_LEN) + 1);

		char *db = NULL;

		for (i = 1; i <= last_write_pos; i += ctx->kv_max_records) {
			db = __get_KV_dbname(i, ctx->kv_max_records - 1);
			return_if_false(db, -1);

			/* 00000000000000000001-00000000000005000000,00000000000005000001-00000000000010000000, */
			strcat(value, db); strcat(value, ",");
			free(db);
		}

		// value[strlen(value)-1] = '\0';
		x_printf(D, "All the databases is:\n\t%s\n", value);
	} else {// Others, such as: XMQ_[PRODUCERS|CONSUMERS]
		value = __load_string_string(ctx, key);
		return_if_false((value != (char *)-1), -1);

		if (value == (char *)1) {
			x_printf(I, "__load_string_string: (Key:%s Val:(char *)1). The Key doesn't exist.", key);
			return 0;
		}
	}

	x_printf(D, "__load_string_string: KEY:<%s> VALUE:<%s>", key, value);

	csv_parser_t csv;
	xmq_csv_parser_init(&csv);
	int fields = xmq_csv_parse_string(&csv, value);
	free(value);
	return_if_false((fields != -1), -1);

	if (-1 == __csv_to_list(ctx, &csv, head, type)) {
		x_printf(E, "__csv_to_list: (type:%s) failed!",
			(type == XMQ_TYPE_DATABASE) ? "XMQ_TYPE_DATABASE" :
			((type == XMQ_TYPE_PRODUCER) ? "XMQ_TYPE_PRODUCER" : "XMQ_TYPE_CONSUMER"));

		xmq_csv_parser_destroy(&csv);
		return 0;
	}

	xmq_csv_parser_destroy(&csv);

	return fields;
}

int __write_back_append(xmq_ctx_t *ctx, const char *key, const char *append, const char *split)
{
	return_if_false((ctx && key && append && split), -1);

	char *value_old = __load_string_string(ctx, key);
	return_if_false((value_old != (char *)-1), -1);
	value_old = (value_old == (char *)1) ? NULL : value_old;

	char *value_new = __string_append_3rd(value_old, split, append, NULL, NULL);

	if (!value_new) {
		x_printf(W, "__string_append_3rd: [prefix:%s,split:%s,append:%s] fail.", value_old, split, append);
		freeif(value_old);
		return -1;
	}

	free(value_old);

	int rc = __write_string_string(ctx, key, value_new);

	if (rc == -1) {
		x_printf(W, "__write_string_string: [Key:%s, Value:%s] fail.", key, value_new);
		freeif(value_new);
		return -1;
	}

	free(value_new);

	return rc;
}

int __write_back_delete(xmq_ctx_t *ctx, const char *key, const char *del)
{
	return_if_false((ctx && key && del), -1);

	char *value_old = __load_string_string(ctx, key);
	return_if_false((value_old != (char *)-1 && value_old != (char *)1), -1);

	/* If not found the substr 'del', return NULL. */
	if (!__string_delete_by_substr(value_old, del)) {
		x_printf(W, "__string_delete_by_substr(src:%s, del:%s) not found the '%s'.", value_old, del, del);
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

int __write_int(xmq_ctx_t *ctx, void *kv_ctx, const char *key, int value)
{
	return __write_string_bin(ctx, kv_ctx, key, (void *)&value, sizeof(int));
}

int __write_uint64(xmq_ctx_t *ctx, void *kv_ctx, const char *key, uint64_t value)
{
	return __write_string_bin(ctx, kv_ctx, key, (void *)&value, sizeof(uint64_t));
}

int __write_string_bin(xmq_ctx_t *ctx, void *kv_ctx, const char *key, const void *value, size_t len)
{
	return_if_false((ctx && kv_ctx && key && value && len), -1);

	bin_entry_t bin_k, bin_v;
	set_bin_entry(&bin_k, key, strlen(key) + 1);
	set_bin_entry(&bin_v, value, len);

	return ctx->kv_put(kv_ctx, &bin_k, &bin_v, 1);
}

int __write_string_string(xmq_ctx_t *ctx, const char *key, const char *value)
{
	return __write_string_bin(ctx, ctx->g_state_ctx, key, value, strlen(value) + 1);
}

int __load_int(xmq_ctx_t *ctx, void *kv_ctx, const char *key, int *value)
{
	size_t  len = 0;
	int     *val = NULL;

	int res = __load_string_bin(ctx, kv_ctx, key, (void **)&val, &len);

	return_if_false((res == 0), res);

	*value = *val;
	free(val);

	return 0;
}

int __load_uint64(xmq_ctx_t *ctx, void *kv_ctx, const char *key, uint64_t *value)
{
	size_t          len = 0;
	uint64_t        *u64 = NULL;

	int res = __load_string_bin(ctx, kv_ctx, key, (void **)&u64, &len);

	return_if_false((res == 0), res);

	*value = *u64;
	free(u64);

	return 0;
}

/* Do not found the value by key, return 1.
 * LevelDB option error return -1.Succed return 0.*/
int __load_string_bin(xmq_ctx_t *ctx, void *kv_ctx, const char *key, void **value, size_t *len)
{
	return_if_false((ctx && kv_ctx && key && value && len), -1);

	bin_entry_t b_key, b_val;
	set_bin_entry(&b_key, key, strlen(key) + 1);

	int res = ctx->kv_get(kv_ctx, &b_key, &b_val, 1);

	if (res != -1) {
		if (b_val.data == NULL) {
			// x_printf(D, "ctx->kv_get(KEY:%s, VAL: null). There isn't this key.", key);
			return 1;
		}

		*value = b_val.data;
		*len = b_val.len;
	}

	return res;
}

char *__load_string_string(xmq_ctx_t *ctx, const char *key)
{
	void *val; size_t len;

	int res = __load_string_bin(ctx, ctx->g_state_ctx, key, &val, &len);

	return (res) ? (char *)res : (char *)val;
}

char *__string_delete_by_substr(char *src, const char *substr)
{
	return_if_false(src && substr, (char *)-1);

	/* Situation: src="master" substr="master" */
	if (strlen(src) == strlen(substr)) {
		return (strcmp(src, substr)) ? NULL : (src[0] = '\0', src);
	}

	char *tmp_str = strdup(src);
	return_if_false(tmp_str, (char *)-1);

	char *pos = strstr(tmp_str, substr);
	return_if_false(pos, NULL);

	int offset = (pos - tmp_str);

	if (offset == 0) {
		/* Situation: src="master,node1,..." substr="master" */
		strcpy(src, tmp_str + strlen(substr));
	} else {
		/* Situation: src="master,node1" substr="node1" */
		if (*(pos + strlen(substr)) == '\0') {
			src[offset] = '\0';
		} else {
			/* Situation: src="master,node1,node2,..." substr="node1" */
			size_t len = strlen(pos + strlen(substr)) + 1;	// +1 for '\0'
			// snprintf(src+offset, len, "%s", pos+strlen(substr));
			strncpy(src + offset, pos + strlen(substr), len);
		}
	}

	x_printf(D, "Before [%s] After [%s].", tmp_str, src);
	free(tmp_str);

	return src;
}

char *__string_append_3rd(const char *prefix, const char *split, const char *first, const char *second, const char *third)
{
	return_if_false(split, (char *)-1);

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

char *__get_KV_dbname(uint64_t start, size_t offset)
{
	char *des = (char *)calloc(1, 2 * XMQ_KEY_LEN);

	return_if_false(des, NULL);

	char s_start[2][XMQ_KEY_LEN];
	__u64toa(start, s_start[0], sizeof(s_start[0]));
	__u64toa(start + offset, s_start[1], sizeof(s_start[0]));
	sprintf(des, "%s-%s", s_start[0], s_start[1]);

	return des;
}

char *__u64toa(uint64_t u64, char *des, int len)
{
	return_if_false((des && len > 0), (char *)-1);

	int i;

	for (i = len - 2; i >= 0; i--) {
		des[i] = 48 + (u64 % 10);
		u64 /= 10;
	}

	des[len - 1] = '\0';

	return des;
}

uint64_t __atou64(const char *str, size_t len)
{
	return_if_false((str && str[0] != '\0' && len > 0), (uint64_t)-1);

	uint64_t        t = 1, des = 0;
	const char      *p = str + len - 1;

	for (; p >= str; p--) {
		des += t * (*p - 48);
		// printf("'%c' [%d] [%llu] des:%llu\n", *p, *p - 48, t*(*p-48), des);
		t *= 10;
	}

	return des;
}

int __mkdir_if_doesnt_exist(const char *dirpath)
{
	return_if_false(dirpath, -1);

	// Check whether exist.
	if (access(dirpath, F_OK) == 0) {
		struct stat st;

		if (stat(dirpath, &st)) {
			x_printf(E, "__mkdir_if_doesnt_exist: stat(%s) fai. Error-%s", dirpath, strerror(errno));
			return -1;
		}

		if (S_ISDIR(st.st_mode)) {
			x_printf(D, "Directory [%s] has already exist.", dirpath);
			return 0;
		}

		x_printf(W, "Never should be here.");
		return 0;
	}

	/* Create directoy. */
	if (mkdir(dirpath, S_IRWXU)) {
		x_printf(E, "mkdir(%s, S_IRWXU) fail. Error-%s", dirpath);
		return -1;
	}

	return 0;
}

