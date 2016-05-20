#include "pool_api.h"
#include "async_api.h"

#include "misc.h"
#include "tsdb.h"
#include "backup.h"
#include "timport_cfg.h"
#include "timport_api.h"

#include "timport_task.h"

#define MAX_SECONDS_FOR_TSDB_READY      5

#define MAX_CONNS_PER_REDIS             200
#define MAX_CONNS_PER_TSDB              100

#ifndef MAX_STEP_COUNT
  #define MAX_STEP_COUNT                5
#endif

#define MAX_REDIS_SIZE                  32

typedef struct
{
	enum
	{
		NO_WORKING = 0,
		IS_WORKING = 1,
	}               work_status;
	timport_task_t  root_ttask;
	time_t          start_tmstamp;
	int             stfd;
	tsdb_t          *tsdb;
	backup_t        *backup;
	int             interval;
} timport_mgr_t;

static timport_mgr_t g_timport_mgr;

extern struct timport_cfg_list g_timport_cfg_list;

static void handle_single_step_index(timport_task_t *p_ttask);

static void handle_next_step_index(timport_task_t *p_ttask);

static void handle_index_cb(struct async_obj *obj, void *reply, void *data);

static void handle_set(timport_task_t *p_ttask);

static void handle_set_cb(struct async_obj *obj, void *reply, void *data);

static void handle_string(timport_task_t *p_ttask);

static void handle_string_cb(struct async_obj *obj, void *reply, void *data);

static inline ASYNC_CALL_BACK get_handle_cb(tkey_type_e type)
{
	ASYNC_CALL_BACK fcb = NULL;

	switch (type)
	{
		case TKEY_TYPE_WHOLE_INDEX:
		case TKEY_TYPE_ALONE_INDEX:
		case TKEY_TYPE_KEYS_INDEX:
			fcb = handle_index_cb;
			break;

		case TKEY_TYPE_STRING_VAL:
			fcb = handle_string_cb;
			break;

		case TKEY_TYPE_SET_VAL:
			fcb = handle_set_cb;
			break;

		default:
			break;
	}

	return fcb;
}

static void async_midway_stop_cb(const struct async_obj *obj, void *data)
{
	struct async_ctx        *ac = (struct async_ctx *)data;
	timport_task_t          *p_ttask = (timport_task_t *)ac->data;

	x_printf(F, "(%s) midway stop", p_ttask->key);

	raise(SIGQUIT);
}

void statistics_finish_cb(const struct async_obj *obj, void *data)
{
	struct async_ctx        *ac = (struct async_ctx *)data;
	struct ev_loop          *loop = (struct ev_loop *)ac->data;

	g_timport_mgr.start_tmstamp += g_timport_mgr.interval;
	set_stime(g_timport_mgr.stfd, (int64_t)g_timport_mgr.start_tmstamp);

	if (ATOMIC_CASB(&(g_timport_mgr.work_status), IS_WORKING, NO_WORKING)) {
		timport_task_check(loop);
		return;
	}
}

void handle_statistics(struct ev_loop *loop, statistics_t *p_stat)
{
	struct async_ctx        *ac = NULL;
	struct cnt_pool         *cpool = NULL;
	void                    *cite = NULL;
	int                     sfd = -1;
	int                     i = 0;
	char                    *proto = NULL;
	int                     len = 0;
	char                    date[32] = { 0 };
	char                    ten_tm[32] = { 0 };
	bool                    has_cmd = false;
	int                     rc = POOL_API_OK;

	ac = async_initial(loop, QUEUE_TYPE_FIFO, statistics_finish_cb, statistics_finish_cb, loop, p_stat->keys_cnt * 2);

	if (NULL == ac) {
		x_printf(E, "async_initial failed");
		goto fail;
	}

	rc = pool_api_gain(&cpool, p_stat->host, p_stat->port, &cite);

	if (POOL_API_OK != rc) {
		x_printf(F, "pool_api_gain failed rc = %d, host = %s, port = %d", rc, p_stat->host, p_stat->port);
		goto fail;
	}

	sfd = (int)(long)cite;

	strftime(date, sizeof(date), "%Y%m%d", localtime(&g_timport_mgr.start_tmstamp));
	strftime(ten_tm, sizeof(ten_tm), "%Y%m%d%H%M", localtime(&g_timport_mgr.start_tmstamp));
	ten_tm[strlen(ten_tm) - 1] = '\0';

	for (i = 0; i < p_stat->keys_cnt; ++i) {
		switch (p_stat->keys[i].mode)
		{
			case STAT_MODE_INCR:

				if (p_stat->keys[i].count > 0) {
					len = cmd_to_proto(&proto, "INCRBY %s:%sCount %lld", date, p_stat->keys[i].name, (long long)p_stat->keys[i].count);

					if (len == 0) {
						x_printf(E, "cmd_to_proto (INCRBY %s:%sCount %lld)", date, p_stat->keys[i].name, (long long)p_stat->keys[i].count);
						break;
					}

					async_command(ac, PROTO_TYPE_REDIS, sfd, cpool, NULL, NULL, proto, (size_t)len);
					free(proto);
					has_cmd = true;
				}

				if (p_stat->keys[i].size > 0) {
					len = cmd_to_proto(&proto, "INCRBY %s:%sSize %lld", date, p_stat->keys[i].name, (long long)p_stat->keys[i].size);

					if (len == 0) {
						x_printf(E, "cmd_to_proto (INCRBY %s:%sSize %lld)", date, p_stat->keys[i].name, (long long)p_stat->keys[i].size);
						break;
					}

					async_command(ac, PROTO_TYPE_REDIS, sfd, cpool, NULL, NULL, proto, (size_t)len);
					free(proto);
					has_cmd = true;
				}

				break;

			case STAT_MODE_SET:

				if (p_stat->keys[i].count > 0) {
					len = cmd_to_proto(&proto, "SET %s:%s %lld", ten_tm, p_stat->keys[i].name, (long long)p_stat->keys[i].count);

					if (len == 0) {
						x_printf(E, "cmd_to_proto (SET %s:%s %lld)", ten_tm, p_stat->keys[i].name, (long long)p_stat->keys[i].count);
						break;
					}

					async_command(ac, PROTO_TYPE_REDIS, sfd, cpool, NULL, NULL, proto, (size_t)len);
					free(proto);
					has_cmd = true;
				}

				break;

			default:
				break;
		}
	}

	if (has_cmd) {
		async_startup(ac);
	} else {
		pool_api_push(cpool, &cite);
		statistics_finish_cb(NULL, (void *)ac);

		if (NULL != ac) {
			async_distory(ac);
		}
	}

	return;

fail:

	if (NULL != ac) {
		async_distory(ac);
	}
}

static void task_finish(timport_task_t *p_ttask)
{
	struct timport_cfg_file *p_cfg = &g_timport_cfg_list.file_info;
	timport_task_t          *p = NULL;

	p = p_ttask->parent;

	if (NULL != p) {
		p->finish_task_count++;

		if (p->next_task_count == p->finish_task_count) {
			handle_next_step_index(p);
		}

		if (NULL == p->parent) {
			x_printf(I, "(%s) total_count = %d, finish_count = %d, next_count = %d\n", p_ttask->key, p->total_task_count, p->finish_task_count, p->next_task_count);
		} else {
			x_printf(D, "(%s) total_count = %d, finish_count = %d, next_count = %d\n", p_ttask->key, p->total_task_count, p->finish_task_count, p->next_task_count);
		}

		if (p->finish_task_count == p->total_task_count) {
			x_printf(I, "(%s) total_count = %d done", p->key, p->finish_task_count);

			if (NULL != p->rds_link) {
				if (TKEY_TYPE_ALONE_INDEX == p->tkey->type) {
					if (NULL != p->reply) {
						handle_set(p);
					} else {
						task_finish(p);
						free(p);
					}
				} else if (TKEY_TYPE_WHOLE_INDEX == p->tkey->type) {
					task_finish(p);
					 free(p);	//free in merge_child_sets
				} else if (TKEY_TYPE_KEYS_INDEX == p->tkey->type) {
					task_finish(p);
					redis_reply_release(p->reply);
					free(p);
				} else {
					x_printf(F, "(%s) invalid tkey_type = %d!", p->key, p->tkey->type);
					raise(SIGQUIT);
				}
			} else {
				if (NULL != p->parent) {
					if (p->tkey->type == TKEY_TYPE_KEYS_INDEX) {
						task_finish(p);
						free(p);
					} else {	// TKEY_TYPE_WHOLE_INDEX
						if (merge_child_sets(p) < 0) {
							goto fail;
						}

						if (NULL != p->reply) {
							p->total_result_count = p->reply->elements;
							handle_set(p);
						} else {
							task_finish(p);
							free(p);
						}
					}
				} else {
					if (p_cfg->statistics.keys_cnt <= 0) {
						g_timport_mgr.start_tmstamp += g_timport_mgr.interval;
						set_stime(g_timport_mgr.stfd, (int64_t)g_timport_mgr.start_tmstamp);

						if (ATOMIC_CASB(&(g_timport_mgr.work_status), IS_WORKING, NO_WORKING)) {
							timport_task_check(p->loop);
							return;
						}
					} else {
						handle_statistics(p->loop, &p_cfg->statistics);
					}
				}
			}
		}
	}

	return;

fail:
	raise(SIGQUIT);
}

static void redis_expire_finish_cb(const struct async_obj *obj, void *data)
{
	struct async_ctx        *ac = (struct async_ctx *)data;
	timport_task_t          *p_ttask = (timport_task_t *)ac->data;

	task_finish(p_ttask);

	redis_reply_release(p_ttask->reply);
	free(p_ttask);
}

static void tsdb_set_finish_cb(const struct async_obj *obj, void *data)
{
	struct async_ctx        *actx = (struct async_ctx *)data;
	timport_task_t          *p_ttask = (timport_task_t *)actx->data;
	struct redis_link       *rds_link = p_ttask->rds_link;
	struct timport_cfg_file *p_cfg = &g_timport_cfg_list.file_info;
	struct async_ctx        *ac = NULL;
	struct cnt_pool         *cpool = NULL;
	char                    *proto = NULL;
	void                    *cite = NULL;
	int                     sfd = -1;
	int                     len = -1;
	int                     i = 0;
	int                     rc = POOL_API_OK;

	static int count = 0;

	x_printf(D, "tsdb_set_end_count = %d", ++count);

	ac = async_initial(p_ttask->loop, QUEUE_TYPE_FIFO, async_midway_stop_cb, redis_expire_finish_cb, p_ttask, p_cfg->redis_cnt);

	if (NULL == ac) {
		x_printf(F, "async_initial failed!");
		goto fail;
	}

	time_t  tl = (time_t)p_ttask->tkey->expire_time;
	time_t  now;
	time(&now);

	if ((g_timport_mgr.start_tmstamp + (time_t)p_cfg->delay_time + tl) < (uint64_t)now) {
		tl = 0;
	} else {
		tl = (g_timport_mgr.start_tmstamp + (time_t)p_cfg->delay_time + tl) - (uint64_t)now;
	}

#ifndef HAVE_TEST
	len = cmd_to_proto(&proto, "EXPIRE %s %d", p_ttask->key, (int)tl);
#else
	len = cmd_to_proto(&proto, "TTL %s", p_ttask->key);
#endif

	if (len < 0) {
		x_printf(F, "cmd_to_proto failed, key = %s", p_ttask->key);
		goto fail;
	}

	if (NULL != rds_link) {
		rc = pool_api_gain(&cpool, rds_link->host, rds_link->port, &cite);

		if (POOL_API_OK != rc) {
			x_printf(F, "pool_api_gain failed rc = %d, host = %s, port = %d", rc, rds_link->host, rds_link->port);
			goto fail;
		}

		sfd = (int)(long)cite;

		async_command(ac, PROTO_TYPE_REDIS, sfd, cpool, NULL, p_ttask, proto, (size_t)len);
	} else {
		for (i = 0; i < p_cfg->redis_cnt; ++i) {
			rds_link = &p_cfg->redis[i];
			rc = pool_api_gain(&cpool, rds_link->host, rds_link->port, &cite);

			if (POOL_API_OK != rc) {
				x_printf(F, "pool_api_gain failed rc = %d, host = %s, port = %d", rc, rds_link->host, rds_link->port);
				goto fail;
			}

			sfd = (int)(long)cite;

			async_command(ac, PROTO_TYPE_REDIS, sfd, cpool, NULL, p_ttask, proto, (size_t)len);
		}
	}

	free(proto);

	async_startup(ac);
	return;

fail:
	raise(SIGQUIT);
}

static void tsdb_set_check_cb(struct async_obj *obj, void *reply, void *data)
{
	struct redis_reply      *r = (struct redis_reply *)reply;
	timport_task_t          *p_ttask = (timport_task_t *)data;

	if ((NULL == r) || (r->type != REDIS_REPLY_STATUS) || (0 != strcasecmp(r->str, "OK")) || (NULL == p_ttask)) {
		x_printf(F, "(%s) tsdb reply err!", p_ttask->key);
		goto fail;
	}

	return;

fail:
	raise(SIGQUIT);
}

static inline int get_tsdb_link(redis_link_t *rlink, int key, tsdb_set_t *p_tsdb)
{
	int             i = 0;
	tsdb_conn_t     *p = NULL;
	int             retry = 1;

	if (NULL == p_tsdb) {
again:
		p = get_tsdb_conn(g_timport_mgr.tsdb, key);

		if (NULL == p) {
			if (retry > 0) {
				retry--;
				goto again;
			} else {
				return -1;
			}
		}

		rlink->host = p->dn.ip;
		rlink->port = p->dn.w_port;
		return 0;
	} else {
		for (i = 0; i < p_tsdb->kset_cnt; ++i) {
			if ((p_tsdb->key_set[i].s_key <= key) && (key < p_tsdb->key_set[i].e_key)) {
				p_tsdb->key_set[i].dn_robin++;
				redis_link_t *q = &p_tsdb->key_set[i].data_node[p_tsdb->key_set[i].dn_robin % p_tsdb->key_set[i].dn_cnt];

				rlink->host = q->host;
				rlink->port = q->port;
				return 0;
			}
		}

		return -1;
	}

	return -1;
}

static void handle_string(timport_task_t *p_ttask)
{
	struct redis_reply      *r = p_ttask->reply;
	int                     sfd = -1;
	struct async_ctx        *ac = NULL;
	struct cnt_pool         *cpool = NULL;
	char                    *proto = NULL;
	int                     len = 0;
	void                    *cite = NULL;
	tsdb_conn_t             *p = NULL;
	tsdb_set_t              *p_tsdb_in_cfg = p_ttask->tkey->tsdb;
	int                     rc = POOL_API_OK;

	static int count = 0;

	x_printf(D, "tsdb_set_start_count = %d", ++count);
	if (NULL == r) {
		x_printf(F, "(%s) reply is null", p_ttask->key);
		goto fail;
	}

	ac = async_initial(p_ttask->loop, QUEUE_TYPE_FIFO, async_midway_stop_cb, tsdb_set_finish_cb, p_ttask, 32);

	if (NULL == ac) {
		x_printf(F, "(%s) async_initial failed!", p_ttask->key);
		goto fail;
	}

	len = cmd_to_proto(&proto, "SET %s %b", p_ttask->key, r->str, r->len);

	if (len < 0) {
		x_printf(F, "cmd_to_proto failed, key = %s", p_ttask->key);
		goto fail;
	}

	if (NULL != p_ttask->param) {
		int key = 0;

		if (p_ttask->tkey->hash_filter) {
			key = p_ttask->tkey->hash_filter(p_ttask->tkey, p_ttask->param, p_ttask->param_len);

			if (key < 0) {
				x_printf(F, "(%s) hash_fiter failed", p_ttask->key);
				goto fail;
			}
		} else {
			x_printf(F, "(%s) no hash_filter", p_ttask->key);
			goto fail;
		}

		redis_link_t rlink;

		if (get_tsdb_link(&rlink, key % 8192, p_tsdb_in_cfg) < 0) {
			x_printf(F, "(%s) get_tsdb_link failed", p_ttask->key);
			goto fail;
		}

		rc = pool_api_gain(&cpool, rlink.host, rlink.port, &cite);

		if (POOL_API_OK != rc) {
			x_printf(F, "(%s) pool_api_gain failed rc = %d, host = %s, port = %d!", p_ttask->key, rc, rlink.host, rlink.port);
			goto fail;
		}

		sfd = (int)(long)cite;

		async_command(ac, PROTO_TYPE_REDIS, sfd, cpool, tsdb_set_check_cb, p_ttask, proto, (size_t)len);
	} else {
		if (NULL == p_tsdb_in_cfg) {
			AO_ThreadSpinLock(&((g_timport_mgr.tsdb)->lock));
			list_for_each_entry(p, &((g_timport_mgr.tsdb)->pool), list)
			{
				rc = pool_api_gain(&cpool, p->dn.ip, p->dn.w_port, &cite);

				if (POOL_API_OK != rc) {
					x_printf(F, "(%s %s) pool_api_gain failed rc = %d, host = %s, port = %d!", p_ttask->tkey->key, p_ttask->ftime, rc, p->dn.ip, p->dn.w_port);
					AO_ThreadSpinUnlock(&((g_timport_mgr.tsdb)->lock));
					goto fail;
				}

				sfd = (int)(long)cite;

				async_command(ac, PROTO_TYPE_REDIS, sfd, cpool, tsdb_set_check_cb, p_ttask, proto, (size_t)len);
			}
			AO_ThreadSpinUnlock(&((g_timport_mgr.tsdb)->lock));
		} else {
			int i = 0;

			for (i = 0; i < p_tsdb_in_cfg->kset_cnt; ++i) {
				redis_link_t *q = &p_tsdb_in_cfg->key_set[i].data_node[0];
				rc = pool_api_gain(&cpool, q->host, q->port, &cite);

				if (POOL_API_OK != rc) {
					x_printf(F, "(%s %s) pool_api_gain failed rc = %d, host = %s, port = %d!", p_ttask->tkey->key, p_ttask->ftime, rc, q->host, q->port);
					goto fail;
				}

				sfd = (int)(long)cite;

				async_command(ac, PROTO_TYPE_REDIS, sfd, cpool, tsdb_set_check_cb, p_ttask, proto, (size_t)len);
			}
		}
	}

	free(proto);
	async_startup(ac);

	if (NULL != g_timport_mgr.backup) {
		backup_set(g_timport_mgr.backup, p_ttask->key, strlen(p_ttask->key), r->str, r->len);
	}

	if (NULL != p_ttask->tkey->stat) {
		p_ttask->tkey->stat->count += p_ttask->total_result_count;
		p_ttask->tkey->stat->size += r->len;
	}

	return;

fail:

	if (NULL == proto) {
		free(proto);
	}

	raise(SIGQUIT);
}

static void handle_string_cb(struct async_obj *obj, void *reply, void *data)
{
	struct redis_reply      *r = (struct redis_reply *)reply;
	timport_task_t          *p_ttask = (timport_task_t *)data;

	if (r->type != REDIS_REPLY_STRING) {
		if (r->type == REDIS_REPLY_NIL) {
			x_printf(W, "(%s) is nil!", p_ttask->key);
			task_finish(p_ttask);
			free(p_ttask);
			return;
		} else {
			x_printf(F, "(%s) get err, type = %d, errstr = %s!", p_ttask->key, r->type, r->str);
			goto fail;
		}
	}

	if ((NULL != p_ttask->tkey) && p_ttask->tkey->result_filter) {
		p_ttask->tkey->result_filter(p_ttask->tkey, &r);
	}

	p_ttask->total_result_count = 1;
	p_ttask->reply = redis_reply_dup(r);
	handle_string(p_ttask);

	return;

fail:
	raise(SIGQUIT);
}

static void handle_set(timport_task_t *p_ttask)
{
	struct redis_reply      *reply_str = NULL;
	int                     ignore_error = 0;

	sort_array(p_ttask->reply);

	if ((NULL != p_ttask->tkey) && p_ttask->tkey->result_filter) {
		p_ttask->tkey->result_filter(p_ttask->tkey, &p_ttask->reply);

		if (p_ttask->total_result_count != p_ttask->reply->elements) {
			p_ttask->total_result_count = p_ttask->reply->elements;
		}
	}

	if ((NULL != p_ttask->tkey) && p_ttask->tkey->ignore_error) {
		ignore_error = 1;
	}

	if (set_to_string(&reply_str, p_ttask->reply, ignore_error) < 0) {
		x_printf(F, "set_to_string failed!");
		goto fail;
	}

	redis_reply_release(p_ttask->reply);

	p_ttask->reply = reply_str;
	handle_string(p_ttask);

	return;

fail:
	raise(SIGQUIT);
}

static void handle_set_cb(struct async_obj *obj, void *reply, void *data)
{
	struct redis_reply      *r = (struct redis_reply *)reply;
	timport_task_t          *p_ttask = (timport_task_t *)data;

	if ((NULL == r) || (NULL == p_ttask)) {
		x_printf(F, "param invalid!");
		goto fail;
	}

	if ((r->type != REDIS_REPLY_ARRAY) && (r->type != REDIS_REPLY_NIL)) {
		x_printf(F, "(%s) smember err, type = %d, errstr = %s", p_ttask->key, r->type, r->str);
		free(p_ttask);
		goto fail;
	}

	if ((REDIS_REPLY_NIL == r->type) || (0 == r->elements)) {
		x_printf(W, "(%s) reply is nil", p_ttask->key);
		//task_finish(p_ttask);
		free(p_ttask);
		return;
	}

	p_ttask->total_result_count = r->elements;
	p_ttask->reply = redis_reply_dup(r);
	handle_set(p_ttask);

	return;

fail:
	raise(SIGQUIT);
}

static void handle_single_step_index(timport_task_t *p_ttask)
{
	int                     i = 0, j = 0;
	void                    *cite = NULL;
	int                     sfd = 0;
	struct async_ctx        *ac = NULL;
	struct cnt_pool         *cpool = NULL;
	int                     sfd_list[MAX_REDIS_SIZE] = { 0 };
	int                     rc = POOL_API_OK;

	struct ev_loop          *loop = p_ttask->loop;
	redis_link_t            *rds_link = p_ttask->rds_link;
	timport_key_t           *p_tkey = p_ttask->tkey;
	struct timport_cfg_file *p_cfg = &g_timport_cfg_list.file_info;

	memset(sfd_list, 0xff, sizeof(sfd_list));

	ac = async_initial(loop, QUEUE_TYPE_FIFO, async_midway_stop_cb, NULL, p_ttask, p_ttask->step * p_tkey->child_cnt);

	if (NULL == ac) {
		x_printf(F, "(%s) async_initial failed!", p_ttask->key);
		goto fail;
		return;
	}

	rc = pool_api_gain(&cpool, rds_link->host, rds_link->port, &cite);

	if (POOL_API_OK != rc) {
		x_printf(F, "(%s) pool_api_gain failed rc = %d, host = %s, port = %d!", p_ttask->key, rc, rds_link->host, rds_link->port);
		goto fail;
	}

	sfd = (int)(long)cite;
	sfd_list[rds_link->idx] = sfd;

	for (i = 0; i < p_ttask->step; ++i) {
		for (j = 0; j < p_tkey->child_cnt; ++j) {
			timport_task_t *p_new_ttask = NULL;
			NewArray0(1, p_new_ttask);

			if (NULL == p_new_ttask) {
				x_printf(F, "(%s) NewArray0 failed!", p_ttask->key);
				goto fail;
			}

			p_new_ttask->tkey = &p_tkey->child[j];
			p_new_ttask->loop = p_ttask->loop;
			p_new_ttask->rds_link = p_ttask->rds_link;
			p_new_ttask->param = p_ttask->reply->element[p_ttask->finish_result_count + i]->str;
			p_new_ttask->param_len = p_ttask->reply->element[p_ttask->finish_result_count + i]->len;
			p_new_ttask->parent = p_ttask;
			p_new_ttask->ftime = p_ttask->ftime;

			if (fmt_task_key(p_new_ttask) != 0) {
				x_printf(F, "(%s -> %s) fmt_task_key failed!", p_ttask->key, p_new_ttask->param);
				goto fail;
			}

			if (p_new_ttask->tkey->redis_filter) {
				int idx = p_new_ttask->tkey->redis_filter(p_new_ttask->tkey, p_new_ttask->param, p_new_ttask->param_len, p_cfg->redis_cnt);

				if (sfd_list[idx] < 0) {
					rc = pool_api_gain(&cpool, p_cfg->redis[idx].host, p_cfg->redis[idx].port, &cite);

					if (POOL_API_OK != rc) {
						x_printf(F, "(%s) pool_api_gain failed, host = %s, port = %d!", p_ttask->key, p_cfg->redis[idx].host, p_cfg->redis[idx].port);
						goto fail;
					}

					sfd = (int)(long)cite;
					sfd_list[idx] = sfd;
				} else {
					sfd = sfd_list[idx];
				}
			}

			char    *proto = NULL;
			int     len = 0;

			if (p_tkey->child[j].type == TKEY_TYPE_STRING_VAL) {
				len = cmd_to_proto(&proto, "GET %s", p_new_ttask->key);
			} else {
				len = cmd_to_proto(&proto, "SMEMBERS %s", p_new_ttask->key);
			}

			if (len <= 0) {
				x_printf(F, "cmd_to_proto failed, key = %s", p_new_ttask->key);
				goto fail;
			}

			ASYNC_CALL_BACK fcb = get_handle_cb(p_tkey->child[j].type);

			if (NULL == fcb) {
				x_printf(F, "(%s) get_handle_cb failed!", p_ttask->key);
				goto fail;
			}

			async_command(ac, PROTO_TYPE_REDIS, sfd, cpool, fcb, p_new_ttask, proto, (size_t)len);
			free(proto);
		}
	}

	async_startup(ac);

	return;

fail:
	raise(SIGQUIT);
}

static void handle_next_step_index(timport_task_t *p_ttask)
{
	p_ttask->finish_result_count += p_ttask->step;

	if (p_ttask->finish_result_count >= p_ttask->total_result_count) {
		return;
	}

	if (p_ttask->total_result_count - p_ttask->finish_result_count < p_ttask->step) {
		p_ttask->step = p_ttask->total_result_count - p_ttask->finish_result_count;
	}

	p_ttask->next_task_count += p_ttask->step * p_ttask->tkey->child_cnt;

	handle_single_step_index(p_ttask);
}

static void handle_index_cb(struct async_obj *obj, void *reply, void *data)
{
	struct redis_reply      *r = (struct redis_reply *)reply;
	timport_task_t          *p_ttask = (timport_task_t *)data;

	if ((NULL == r) || (NULL == p_ttask)) {
		x_printf(F, "param invalid!");
		goto fail;
	}

	if (((REDIS_REPLY_ARRAY != r->type) && (REDIS_REPLY_NIL != r->type)) || (p_ttask->tkey->child_cnt <= 0)) {
		x_printf(F, "(%s) err, type = %d", p_ttask->key, r->type);
		free(p_ttask);
		goto fail;
	}

	if ((REDIS_REPLY_NIL == r->type) || (0 == r->elements)) {
		x_printf(W, "(%s) reply is nil", p_ttask->key);
		//task_finish(p_ttask);
		free(p_ttask);
		return;
	}

	p_ttask->reply = redis_reply_dup(r);
	p_ttask->total_result_count = (int)r->elements;
	p_ttask->finish_result_count = 0;
	p_ttask->total_task_count = (int)r->elements * p_ttask->tkey->child_cnt;
	p_ttask->finish_task_count = 0;
	p_ttask->step = MIN((int)r->elements, MAX_STEP_COUNT);

	p_ttask->next_task_count += p_ttask->step * p_ttask->tkey->child_cnt;

	handle_single_step_index(p_ttask);

	return;

fail:
	raise(SIGQUIT);
}

static void timport_task_start(struct ev_loop *loop, time_t tmstamp)
{
	struct timport_cfg_file *p_cfg = &g_timport_cfg_list.file_info;
	timport_key_t           *p_tktree = &p_cfg->tktree;
	timport_task_t          *p_root_ttask = &g_timport_mgr.root_ttask;

	struct async_ctx        *ac = NULL;
	struct cnt_pool         *cpool = NULL;
	int                     sfd = -1;
	void                    *cite = NULL;
	int                     i = 0;
	int                     j = 0;
	int                     rc = POOL_API_OK;

	for (i = 0; i < p_cfg->statistics.keys_cnt; ++i) {
		p_cfg->statistics.keys[i].count = 0;
		p_cfg->statistics.keys[i].size = 0;
	}

	p_root_ttask->loop = loop;
	p_root_ttask->finish_task_count = 0;
	p_root_ttask->total_task_count = 0;

	for (i = 0; i < p_tktree->child_cnt; ++i) {
		if (p_cfg->has_ten_min && (p_tktree->child[i].interval == TKEY_INTERVAL_ONE_HOUR) && (tmstamp % 3600 != 0)) {
			x_printf(W, "key: %s is not import now, tmstamp = %d", p_tktree->child[i].key, (int)tmstamp);
			continue;
		}

		timport_task_t *p_secondary_ttask = NULL;
		NewArray0(1, p_secondary_ttask);

		if (NULL == p_secondary_ttask) {
			x_printf(F, "NewArray0 failed, key = %s", p_tktree->child[i].key);
			goto fail;
		}

		p_secondary_ttask->parent = p_root_ttask;
		timport_task_t *p_last_ttask = NULL;

		for (j = 0; j < p_cfg->redis_cnt; ++j) {
			char *ftime = get_ftime(tmstamp, p_tktree->child[i].interval);

			if (NULL == ftime) {
				x_printf(F, "get_ftime failed, key = %s, interval = %d", p_tktree->child[i].key, (int)p_tktree->child[i].interval);
				goto fail;
			}

			timport_task_t *p_ttask = NULL;
			NewArray0(1, p_ttask);
			if (NULL == p_ttask) {
				x_printf(F, "NewArray0 failed, key = %s", p_tktree->child[i].key);
				goto fail;
			}

			p_secondary_ttask->total_task_count++;

			p_ttask->tkey = &p_tktree->child[i];
			p_ttask->loop = loop;
			p_ttask->rds_link = &p_cfg->redis[j];
			p_ttask->ftime = ftime;
			p_ttask->parent = p_secondary_ttask;

			if (fmt_task_key(p_ttask) != 0) {
				x_printf(F, "fmt_task_key failed!");
				goto fail;
			}

			if (NULL == p_secondary_ttask->child) {
				p_secondary_ttask->child = p_ttask;
				p_secondary_ttask->loop = p_ttask->loop;
				p_secondary_ttask->tkey = p_ttask->tkey;
				strcpy(p_secondary_ttask->key, p_ttask->key);
			} else {
				p_last_ttask->slibing = p_ttask;
			}

			p_last_ttask = p_ttask;

			ac = async_initial(loop, QUEUE_TYPE_FIFO, async_midway_stop_cb, NULL, p_ttask, 1);

			if (NULL == ac) {
				x_printf(F, "(%s) async_initial failed!", p_ttask->key);
				goto fail;
			}

			rc = pool_api_gain(&cpool, p_cfg->redis[j].host, p_cfg->redis[j].port, &cite);

			if (POOL_API_OK != rc) {
				x_printf(F, "(%s) pool_api_gain failed rc = %d, host = %s, port = %d", p_ttask->key, rc, p_cfg->redis[j].host, p_cfg->redis[j].port);
				goto fail;
			}

			sfd = (int)(long)cite;

			char    *proto = NULL;
			int     len = 0;

			if (p_tktree->child[i].type == TKEY_TYPE_STRING_VAL) {
				len = cmd_to_proto(&proto, "GET %s", p_ttask->key);
			} else if (p_tktree->child[i].type == TKEY_TYPE_KEYS_INDEX) {
				len = cmd_to_proto(&proto, "KEYS %s", p_ttask->key);
			} else {
				len = cmd_to_proto(&proto, "SMEMBERS %s", p_ttask->key);
			}

			if (len <= 0) {
				x_printf(F, "(%s) cmd_to_proto failed!", p_ttask->key);
				goto fail;
			}

			async_command(ac, PROTO_TYPE_REDIS, sfd, cpool, handle_index_cb, p_ttask, proto, (size_t)len);
			free(proto);

			async_startup(ac);
		}

		p_root_ttask->total_task_count++;
	}

	return;

fail:
	raise(SIGQUIT);
}

void timport_task_init(void)
{
	int                     i = 0;
	int                     j = 0;
	struct timport_cfg_file *p_cfg = &g_timport_cfg_list.file_info;

	memset(&g_timport_mgr, 0, sizeof(g_timport_mgr));

	if (p_cfg->has_ten_min) {
		g_timport_mgr.interval = 600;
	} else {
		g_timport_mgr.interval = 3600;
	}

	if (!p_cfg->zk_disabled) {
		g_timport_mgr.tsdb = tsdb_open(p_cfg->zk_servers, p_cfg->zk_rnode);

		if (NULL == g_timport_mgr.tsdb) {
			x_printf(F, "tsdb_open failed!");
			raise(SIGQUIT);
			return;
		}
	}

	g_timport_mgr.backup = backup_open(p_cfg->backup_path);

	if (NULL == g_timport_mgr.backup) {
		x_printf(F, "backup_open failed!");
		raise(SIGQUIT);
		return;
	}

	g_timport_mgr.stfd = open(p_cfg->start_time_file, O_RDWR, S_IRUSR | S_IWUSR);

	if (g_timport_mgr.stfd < 0) {
		x_printf(F, "start_time_file open failed!");
		raise(SIGQUIT);
		return;
	}

	g_timport_mgr.start_tmstamp = (time_t)get_stime(g_timport_mgr.stfd);

	/* redis connection pool init */
	for (i = 0; i < p_cfg->redis_cnt; ++i) {
		if (!pool_api_init(p_cfg->redis[i].host, p_cfg->redis[i].port, MAX_CONNS_PER_REDIS, true)) {
			raise(SIGQUIT);
			return;
		}
	}

	if (p_cfg->statistics.keys_cnt > 0) {
		if (!pool_api_init(p_cfg->statistics.host, p_cfg->statistics.port, MAX_CONNS_PER_REDIS, true)) {
			raise(SIGQUIT);
			return;
		}
	}

	for (i = 0; i < p_cfg->tsdb.kset_cnt; ++i) {
		for (j = 0; j < p_cfg->tsdb.key_set[i].dn_cnt; ++j) {
			if (!pool_api_init(p_cfg->tsdb.key_set[i].data_node[j].host, p_cfg->tsdb.key_set[i].data_node[j].port, MAX_CONNS_PER_TSDB, true)) {
				raise(SIGQUIT);
				return;
			}
		}
	}

	/* check tsdb is whether ready ? */
	if (!p_cfg->zk_disabled) {
		i = 0;

		while ((!tsdb_is_ready(g_timport_mgr.tsdb)) && (i < MAX_SECONDS_FOR_TSDB_READY)) {
			struct timespec req = {
				.tv_sec = 1,
			};

			nanosleep(&req, NULL);
			i++;
		}

		if (i == MAX_SECONDS_FOR_TSDB_READY) {
			printf("QUIT: after %ds, tsdb is steal not ready!\n", MAX_SECONDS_FOR_TSDB_READY);
			x_printf(F, "after %ds, tsdb is steal not ready!", MAX_SECONDS_FOR_TSDB_READY);
			raise(SIGQUIT);
			return;
		}
	}
}

void timport_task_exit(void)
{
	backup_close(g_timport_mgr.backup);
	tsdb_close(g_timport_mgr.tsdb);

	if (g_timport_mgr.stfd >= 0) {
		close(g_timport_mgr.stfd);
	}
}

void timport_task_check(void *user)
{
	struct timport_cfg_file *p_cfg = &g_timport_cfg_list.file_info;
	struct ev_loop          *loop = (struct ev_loop *)user;
	time_t                  now;

	time(&now);

	x_printf(I, "=== (work_status = %d, start_tmstamp = %d, diff_time = %d, delay_time = %d) ===", (int)g_timport_mgr.work_status, (int)g_timport_mgr.start_tmstamp, (int)(now - g_timport_mgr.start_tmstamp), p_cfg->delay_time);

	if (now - g_timport_mgr.start_tmstamp > (time_t)p_cfg->delay_time) {
		if (ATOMIC_CASB(&(g_timport_mgr.work_status), NO_WORKING, IS_WORKING)) {
			x_printf(I, "====== timport_task_start(%d) ======", (int)g_timport_mgr.start_tmstamp);
			timport_task_start(loop, g_timport_mgr.start_tmstamp);
		}
	}
}

