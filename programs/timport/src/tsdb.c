#include <signal.h>

#include "utils.h"
#include "pool_api.h"

#include "tsdb.h"

#define TSDB_SIZE               8192

#define MAX_CONNS_PER_TSDB      100

static void add_dn_to_pool(tsdb_t *tsdb, data_node_t *dn)
{
	struct list_head        *pool = &tsdb->pool;
	tsdb_conn_t             *tsdb_conn = NULL;
	int                     rc = POOL_API_OK;

	if ((dn->parent->mode == RW) && (dn->parent->e_time == (uint64_t)(-1))) {
		x_printf(I, "[ADD] mode:%s, ip: %s, wport: %d, rport: %d, role: %s",
			dn->parent->mode == RW ? "RW" : "RO", dn->ip, dn->w_port, dn->r_port, dn->role);

		list_for_each_entry(tsdb_conn, pool, list)
		{
			if (memcmp(dn, &tsdb_conn->dn, sizeof(data_node_t)) == 0) {
				break;
			}
		}

		if (&tsdb_conn->list == pool) {
			tsdb_conn_t *p = NULL;
			NewArray0(1, p);

			if (NULL == p) {
				x_printf(F, "NewArray0 failed!");
				raise(SIGQUIT);
				return;
			}

			if (tsdb->status == NO_READY) {
				if (!conn_xpool_init(dn->ip, dn->w_port, MAX_CONNS_PER_TSDB, true)) {
					x_printf(F, "conn_xpool_init failed, host = %s, port = %d", dn->ip, dn->w_port);
					raise(SIGQUIT);
					return;
				}
			} else {
				struct cnt_pool *cpool = NULL;
				void            *cite = NULL;
				rc = conn_xpool_gain(&cpool, dn->ip, dn->w_port, &cite);

				if (POOL_API_OK != rc) {
					if (!conn_xpool_init(dn->ip, dn->w_port, MAX_CONNS_PER_TSDB, true)) {
						x_printf(F, "conn_xpool_init failed, host = %s, port = %d", dn->ip, dn->w_port);
						raise(SIGQUIT);
						return;
					}
				} else {
					conn_xpool_push(cpool, &cite);
				}
			}

			AO_ThreadSpinLock(&tsdb->lock);

			dn->ctx = (void *)p;

			memcpy(&p->dn, dn, sizeof(data_node_t));

			list_add_tail(&p->list, pool);

			AO_ThreadSpinUnlock(&tsdb->lock);

			AO_CASB(&tsdb->status, NO_READY, IS_READY);
		}
	}
}

static void del_dn_from_pool(tsdb_t *tsdb, data_node_t *dn)
{
	struct list_head        *pool = &tsdb->pool;
	tsdb_conn_t             *tsdb_conn = NULL;
	tsdb_conn_t             *next = NULL;

	if ((dn->parent->mode == RW) && (dn->parent->e_time == (uint64_t)(-1))) {
		x_printf(I, "[DEL] mode:%s, ip: %s, wport: %d, rport: %d, role: %s, ctx: %p",
			dn->parent->mode == RW ? "RW" : "RO", dn->ip, dn->w_port, dn->r_port, dn->role, dn->ctx);

		list_for_each_entry_safe(tsdb_conn, next, pool, list)
		{
			if (memcmp(dn, &tsdb_conn->dn, sizeof(data_node_t)) == 0) {
				AO_ThreadSpinLock(&tsdb->lock);

				dn->ctx = (void *)NULL;

				list_del(&tsdb_conn->list);

				AO_ThreadSpinUnlock(&tsdb->lock);

				// TODO: have no idea of freeing pool of the deleted dn
#if 0
				struct cnt_pool *cpool = NULL;
				void            *cite = NULL;

				while (POOL_API_OK == conn_xpool_gain(&cpool, dn->ip, dn->w_port, &cite)) {
					x_printf(I, "(%s:%d) ==> conn_xpool_free (cite = %d)", dn->ip, (int)dn->w_port, (int)(long)cite);
					conn_xpool_free(cpool, &cite);
				}
#endif

				free(tsdb_conn);

				break;
			}
		}
	}
}

static void pool_init(struct list_head *pool)
{
	INIT_LIST_HEAD(pool);
}

static void pool_free(struct list_head *pool)
{
	tsdb_conn_t *p, *q;

	list_for_each_entry_safe(p, q, pool, list)
	{
		free(p);
	}
}

/* rnode == "/tsdb" or "/msg" */
tsdb_t *tsdb_open(const char *zk_servers, const char *rnode)
{
	int     ret;
	tsdb_t  *tsdb = NULL;

	NewArray0(1, tsdb);

	if (NULL == tsdb) {
		x_printf(F, "NewArray0 error.");
		return NULL;
	}

	AO_ThreadSpinLockInit(&tsdb->lock, false);

	pool_init(&tsdb->pool);

	tsdb->zc = open_zkhandler(zk_servers, (alloc_dn_hook_f)add_dn_to_pool, (free_dn_hook_f)del_dn_from_pool, (void *)tsdb);

	if (NULL == tsdb->zc) {
		x_printf(F, "open_zkhandler error.");
		pool_free(&tsdb->pool);

		free(tsdb);
		return NULL;
	}

	ret = register_zkcache(tsdb->zc, rnode);

	if (ret != 0) {
		x_printf(F, "register_zkcache error.");
		unregister_zkcache(tsdb->zc);
		close_zkhandler(tsdb->zc);
		free(tsdb);
		return NULL;
	}

	return tsdb;
}

void tsdb_close(tsdb_t *tsdb)
{
	if (NULL != tsdb) {
		if (NULL != tsdb->zc) {
			unregister_zkcache(tsdb->zc);
			close_zkhandler(tsdb->zc);
		}

		pool_free(&tsdb->pool);

		free(tsdb);
	}
}

bool tsdb_is_ready(tsdb_t *tsdb)
{
	if (NULL != tsdb) {
		return tsdb->status == IS_READY;
	}

	return false;
}

tsdb_conn_t *get_tsdb_conn(tsdb_t *tsdb, int key)
{
	data_set_t      *ds;
	int             ret;
	tsdb_conn_t     *p = NULL;

	if (key < 0) {
		return NULL;
	}

	key = key % TSDB_SIZE;

	AO_ThreadSpinLock(&tsdb->lock);

	ret = get_write_dataset(tsdb->zc, (uint64_t)key, (const data_set_t **)&ds);

	if ((ret != 0) || (ds->dn_cnt <= 0)) {
		AO_SpinUnlock(&tsdb->lock);
		return NULL;
	}

	p = (tsdb_conn_t *)(ds->data_node[(ds->dn_idx++) % ds->dn_cnt].ctx);

	AO_ThreadSpinUnlock(&tsdb->lock);

	return p;
}

