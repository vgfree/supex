/*
 * file: zk.c
 * date: 2014-08-04
 * auth: chenjianfei@daoke.me
 * desc: zk.c
 */

#include <string.h>
#include <stdio.h>

#include "utils.h"
#include "zk.h"

#define get_field()		\
	p = strchr(p, ':') + 1;	\
	if (p == NULL) {	\
		return -1;	\
	}			\
	snprintf(tmp, strchr(p, ':') - p + 1, "%s", p);

static int register_expansion_zkcache(zhandle_t *zkhandle, expansion_set_t *es);

static int unregister_expansion_zkcache(expansion_set_t *es);

static int register_keyset_zkcache(zhandle_t *zh, const char *path, key_set_t *key_set);

static int unregister_keyset_zkcache(key_set_t *ks);

/*
 * zk-server watcher.
 */
static void zk_watcher(zhandle_t *zh, int type, int state, const char *path, void *zk_context)
{
	if (type == ZOO_SESSION_EVENT) {
		if (state == ZOO_CONNECTED_STATE) {
			x_printf(I, "[zk_watcher]:connected to zookeeper service successfully!\n");
		} else if (state == ZOO_EXPIRED_SESSION_STATE) {
			printf("[luazk][zk_watcher]:Zookeeper session expired!\n");

			zk_ctx_t *zc = (zk_ctx_t *)zk_context;
			zc->zkhandle = zookeeper_init(zc->zkhost, zk_watcher, 30000, 0, zc, 0);

			// FIXME: need double check.
			x_printf(F, "[zk_watcher]:Zookeeper session expired!");
			raise(SIGQUIT);
		}
	}
}

/*
 * watch: "/tsdb"
 */
static void tsdb_watcher(zhandle_t *zh, int type, int state, const char *path, void *zk_cxt)
{
	x_printf(I, "[znode_watcher]:type: %d, state: %d, path: %s\n", type, state, path);

	if (state == ZOO_CONNECTED_STATE) {
		/* we watched znode is changed. */
		if (type == ZOO_CHILD_EVENT) {
			zk_ctx_t *zc = (zk_ctx_t *)zk_cxt;

			unregister_zkcache(zc);

			// need register again FIXME: check.
			register_zkcache(zc, path);
		}
	}
}

/*
 * watch: "/tsdb/RW:20150101000000:-1"
 */
static void expansion_watcher(zhandle_t *zh, int type, int state, const char *path, void *es_cxt)
{
	x_printf(I, "[znode_watcher]:type: %d, state: %d, path: %s\n", type, state, path);

	if (state == ZOO_CONNECTED_STATE) {
		/* we watched znode is changed. */
		if (type == ZOO_CHILD_EVENT) {
			expansion_set_t *es = (expansion_set_t *)es_cxt;

			unregister_expansion_zkcache(es);

			// need register again FIXME: check.
			register_expansion_zkcache(zh, es);
		}
	}
}

/*
 * watch: "/tsdb/RW:20150101000000:-1/0:2048" ...
 */
static void keyset_watcher(zhandle_t *zh, int type, int state, const char *path, void *key_set)
{
	if (state == ZOO_CONNECTED_STATE) {
		if (type == ZOO_CHILD_EVENT) {
			key_set_t *ks = (key_set_t *)key_set;
			unregister_keyset_zkcache(ks);
			register_keyset_zkcache(zh, path, ks);
		}
	}
}

zk_ctx_t *open_zkhandler(const char *zkhost, alloc_dn_hook_f alloc_dn_hook, free_dn_hook_f free_dn_hook, void *ctx)
{
	/* alloc zk context. */
	zk_ctx_t *zc = (zk_ctx_t *)malloc(sizeof(zk_ctx_t));

	if (zc == NULL) {
		return NULL;
	}

	bzero(zc, sizeof(zk_ctx_t));

	/* set zk log level. */
	zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
	strcpy(zc->zkhost, zkhost);

	/* open a zookeeper. */
	zc->zkhandle = zookeeper_init(zkhost, zk_watcher, 30000, 0, zc, 0);

	if (zc->zkhandle == NULL) {
		free(zc);
		return NULL;
	}

	zc->alloc_dn_hook = alloc_dn_hook;
	zc->free_dn_hook = free_dn_hook;
	zc->ctx = ctx;

	return zc;
}

int close_zkhandler(zk_ctx_t *zc)
{
	/* check handle. */
	if ((zc == NULL) || (zc->zkhandle == NULL)) {
		return -1;
	}

	/* close zk-server. */
	zookeeper_close(zc->zkhandle);

	/* free resource. */
	free(zc);

	return 0;
}

static int parse_expansion_node(const char *node, expansion_set_t *expansion_set)
{
	char tmp[64] = { 0 };

	/* mode */
	const char *p = node;

	if (strstr(p, "RW:") != NULL) {
		expansion_set->mode = RW;
	} else if (strstr(p, "RO:") != NULL) {
		expansion_set->mode = RO;
	} else {
		return -1;
	}

	/* start time */
	get_field();
	expansion_set->s_time = atoll(tmp);

	/* end time */
	get_field();
	expansion_set->e_time = atoll(tmp);

	return 0;
}

/*
 * keynode: "0:2048"
 */
static int parse_keyset_node(const char *node, uint64_t *sk, uint64_t *ek)
{
	char *stop = NULL;

	*sk = strtoll(node, &stop, 10);

	if (stop == NULL) {
		return -1;
	}

	*ek = strtoll(strchr(node, ':') + 1, &stop, 10);

	if (stop == NULL) {
		return -1;
	}

	return 0;
}

static int register_expansion_zkcache(zhandle_t *zkhandle, expansion_set_t *es)
{
	/* check parameter */
	if ((zkhandle == NULL) || (es == NULL)) {
		return -1;
	}

	/* no need refresh */
	if (es->key_cnt != 0) {
		return 0;
	}

	/* clean the keysets. */
	bzero(es->key_set, sizeof(es->key_set));

	/*
	 * get "/tsdb/" children and set watcher.
	 */
	struct String_vector sv;
	bzero(&sv, sizeof(sv));
	int32_t ret = zoo_wget_children(zkhandle, es->path, expansion_watcher, es, &sv);

	if ((ret != ZOK) || (sv.count == 0) || (sv.count > EXPANSION_SIZE)) {
		deallocate_String_vector(&sv);
		return -1;
	}

	/* set keysets. */
	int32_t i;

	for (i = 0; i < sv.count; ++i) {
		sprintf(es->key_set[i].path, "%s/%s", es->path, sv.data[i]);
		ret = parse_keyset_node(sv.data[i], &(es->key_set[i].s_key), &(es->key_set[i].e_key));

		if (ret == -1) {
			deallocate_String_vector(&sv);
			return -1;
		}

		es->key_set[i].parent = es;

		es->key_cnt++;
#if 1
		ret = register_keyset_zkcache(zkhandle, es->key_set[i].path, &(es->key_set[i]));

		if (ret == -1) {
			deallocate_String_vector(&sv);
			return -1;
		}
		es->key_set[i].is_synced = 1;
#endif
	}

	deallocate_String_vector(&sv);

	return 0;
}

static int unregister_expansion_zkcache(expansion_set_t *es)
{
	int i;

	if (es->parent->free_dn_hook != NULL) {
		for (i = 0; i < es->key_cnt; ++i) {
			unregister_keyset_zkcache(&es->key_set[i]);
		}
	}

	es->key_cnt = 0;
	return 0;
}

int register_zkcache(zk_ctx_t *zc, const char *rnode)
{
	/* check handle. */
	if ((zc == NULL) || (zc->zkhandle == NULL)) {
		return -1;
	}

	/* no need refresh. */
	if (zc->expansion_cnt != 0) {
		return 0;
	}

	/* clean the keysets. */
	bzero(zc->expansion_set, sizeof(zc->expansion_set));

	/*
	 * get "/tsdb" children and set watcher.
	 */
	struct String_vector sv;
	bzero(&sv, sizeof(sv));
	int32_t ret = zoo_wget_children(zc->zkhandle, rnode, tsdb_watcher, zc, &sv);

	if ((ret != ZOK) || (sv.count == 0) || (sv.count > EXPANSION_SIZE)) {
		deallocate_String_vector(&sv);
		return -1;
	}

	int32_t i;

	for (i = 0; i < sv.count; ++i) {
		sprintf(zc->expansion_set[i].path, "%s/%s", rnode, sv.data[i]);

		ret = parse_expansion_node(sv.data[i], &zc->expansion_set[i]);

		if (ret == -1) {
			deallocate_String_vector(&sv);
			return -1;
		}

		zc->expansion_set[i].parent = zc;

		ret = register_expansion_zkcache(zc->zkhandle, &zc->expansion_set[i]);

		if (ret == -1) {
			deallocate_String_vector(&sv);
			return -1;
		}

		zc->expansion_cnt++;

		if (zc->expansion_set[i].mode == RW) {
			zc->expansion_rw_idx = i;
		}
	}

	deallocate_String_vector(&sv);

	return 0;
}

int unregister_zkcache(zk_ctx_t *zc)
{
	int i;

	if (zc->free_dn_hook != NULL) {
		for (i = 0; i < zc->expansion_cnt; ++i) {
			unregister_expansion_zkcache(&zc->expansion_set[i]);
		}
	}

	zc->expansion_cnt = 0;
	return 0;
}

/*
 * node: RW:1:MASTER:192.168.1.12:7503:7504:20140101000000:-1
 * or:   RO:1:SLAVE:192.168.1.12:7503:7504:20130101000000:20140101000000
 */
static int parser_datanode(const char *node, data_set_t *data_set)
{
	char tmp[64] = { 0 };

	/* mode */
	const char *p = node;

	if (strstr(p, "RW:") != NULL) {
		data_set->mode = RW;
	} else if (strstr(p, "RO:") != NULL) {
		data_set->mode = RO;
	} else {
		return -1;
	}

	/* ds_id */
	get_field();
	data_set->id = atoi(tmp);

	/* role */
	get_field();

	if (strcmp(tmp, MASTER) == 0) {
		data_set->data_node[0].role = MASTER;
	} else if (strcmp(tmp, SLAVE) == 0) {
		data_set->data_node[0].role = SLAVE;
	} else {
		data_set->data_node[0].role = ERRROLE;
	}

	/* ip */
	get_field();
	strcpy(data_set->data_node[0].ip, tmp);

	/* write port */
	get_field();
	data_set->data_node[0].w_port = atoi(tmp);

	/* read port */
	get_field();
	data_set->data_node[0].r_port = atoi(tmp);

	/* start time */
	get_field();
	data_set->s_time = atoll(tmp);

	/* end time */
	get_field();
	data_set->e_time = atoll(tmp);

	return 0;
}

/*
 * path: /keys/0:1024
 */
int register_keyset_zkcache(zhandle_t *zh, const char *path, key_set_t *key_set)
{
	/* check args. */
	if ((key_set == NULL) || (key_set->is_synced != 0)) {
		return 0;
	}

	/* get keys's children node and set watcher. */
	struct String_vector sv;
	bzero(&sv, sizeof(sv));
	int32_t ret = zoo_wget_children(zh, path, keyset_watcher, key_set, &sv);

	if (ret != ZOK) {
		deallocate_String_vector(&sv);
		return -1;
	}

	int             i, j, offset;
	data_set_t      tmp_ds;
	zk_ctx_t        *zc = key_set->parent->parent;

	for (i = 0; i < sv.count; ++i) {
		bzero(&tmp_ds, sizeof(data_set_t));
		ret = parser_datanode(sv.data[i], &tmp_ds);

		if (ret == -1) {
			deallocate_String_vector(&sv);
			return -1;
		}

		for (j = 0; j < key_set->ds_cnt; ++j) {
			if (tmp_ds.id == key_set->data_set[j].id) {
				if (key_set->data_set[j].dn_cnt < DN_PER_DS) {
					offset = key_set->data_set[j].dn_cnt;
					memcpy(&(key_set->data_set[j].data_node[offset]), &tmp_ds.data_node[0], sizeof(data_node_t));
					key_set->data_set[j].data_node[offset].parent = &(key_set->data_set[j]);
					key_set->data_set[j].dn_idx = 0;
					key_set->data_set[j].dn_cnt++;

					if (NULL != zc->alloc_dn_hook) {
						zc->alloc_dn_hook(zc->ctx, &(key_set->data_set[j].data_node[offset]));
					}

					break;
				} else {
					deallocate_String_vector(&sv);
					return -1;
				}
			}
		}

		if (j == key_set->ds_cnt) {
			if (key_set->ds_cnt < MAX_DS_PER_KEY) {
				memcpy(&(key_set->data_set[j]), &tmp_ds, sizeof(data_set_t));
				key_set->data_set[j].data_node[0].parent = &(key_set->data_set[j]);
				key_set->data_set[j].dn_idx = 0;
				key_set->data_set[j].dn_cnt = 1;
				key_set->ds_cnt++;

				if (NULL != zc->alloc_dn_hook) {
					zc->alloc_dn_hook(zc->ctx, &(key_set->data_set[j].data_node[0]));
				}
			} else {
				deallocate_String_vector(&sv);
				return -1;
			}
		}
	}

	deallocate_String_vector(&sv);
	return 0;
}

int unregister_keyset_zkcache(key_set_t *ks)
{
	int             i, j;
	zk_ctx_t        *zc = ks->parent->parent;

	if (zc->free_dn_hook != NULL) {
		for (i = 0; i < ks->ds_cnt; ++i) {
			for (j = 0; j < ks->data_set[i].dn_cnt; ++j) {
				zc->free_dn_hook(zc->ctx, &(ks->data_set[i].data_node[j]));
			}
		}
	}

	ks->is_synced = 0;
	ks->ds_cnt = 0;

	return 0;
}

int get_write_dataset(zk_ctx_t *zc, uint64_t key_field, const data_set_t **ds)
{
	expansion_set_t *es = NULL;

	/* check handle. */
	if ((zc == NULL) || (zc->zkhandle == NULL)) {
		return -1;
	}

	if (NULL != ds) {
		*ds = NULL;
	}

	if (zc->expansion_cnt == 0) {
		return -1;
	}

	es = &zc->expansion_set[zc->expansion_rw_idx];

	/* check valid. */
	if ((es->mode != RW) || (es->key_cnt == 0)) {
		return -1;
	}

	int i;

	for (i = 0; i < es->key_cnt; ++i) {
		if ((key_field >= es->key_set[i].s_key) && (key_field < es->key_set[i].e_key)) {
			if (es->key_set[i].is_synced == 0) {
				/* need fresh. */
				if (-1 == register_keyset_zkcache(zc->zkhandle, es->key_set[i].path, &(es->key_set[i]))) {
					break;
				}

				es->key_set[i].is_synced = 1;
			}

			if (es->key_set[i].data_set[0].mode != RW) {
				break;
			}

			if (NULL != ds) {
				*ds = &(es->key_set[i].data_set[0]);
			}

			return 0;
		}
	}

	return -1;
}

int get_read_dataset(zk_ctx_t *zc, uint64_t key_field, uint64_t time_field, const data_set_t **ds)
{
	expansion_set_t *es = NULL;

	/* check handle. */
	if ((zc == NULL) || (zc->zkhandle == NULL)) {
		return -1;
	}

	if (NULL != ds) {
		*ds = NULL;
	}

	if (zc->expansion_cnt == 0) {
		return -1;
	}

	int i, j;

	for (i = 0; i < zc->expansion_cnt; i++) {
		if ((time_field >= zc->expansion_set[i].s_time)
			&& (time_field < zc->expansion_set[i].e_time)) {
			es = &zc->expansion_set[i];
			break;
		}
	}

	if ((NULL == es) || (es->key_cnt == 0)) {
		return -1;
	}

	for (i = 0; i < es->key_cnt; ++i) {
		if ((key_field >= es->key_set[i].s_key) && (key_field < es->key_set[i].e_key)) {
			if (es->key_set[i].is_synced == 0) {
				if (-1 == register_keyset_zkcache(zc->zkhandle, es->key_set[i].path, &(es->key_set[i]))) {
					return 0;
				}

				es->key_set[i].is_synced = 1;
			}

			for (j = 0; j < es->key_set[i].ds_cnt; ++j) {
#if 0
				if ((time_field >= es->key_set[i].data_set[j].s_time)
					&& (time_field < es->key_set[i].data_set[j].e_time)) {
#endif

				if (es->key_set[i].data_set[j].mode != NA) {
					if (NULL != ds) {
						*ds = &(es->key_set[i].data_set[j]);
					}

					return 0;
				}

				break;
#if 0
			}
#endif
			}

			break;
		}
	}

	return -1;
}

