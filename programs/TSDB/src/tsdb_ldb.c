#include <string.h>
#include <stdlib.h>

#include "redis_parse.h"
#include "utils.h"
#include "tsdb_cfg.h"
#include "tsdb_ldb.h"

#include "binlog.h"
#include "slave.h"
#include "ldb_priv.h"

#define OPT_NUMBER      ":%d\r\n"
#define OPT_PONG        "+PONG\r\n"
#define OPT_NULL        "$-1\r\n"
#define OPT_EMPTY_MULTI "*0\r\n"

static struct _leveldb_stuff *s_ldb = NULL;

int G_PAGE_SIZE = 0;

short   LDB_READONLY_SWITCH;
time_t  LDB_START_TIME;

int tsdb_ldb_init(char *db_name, struct tsdb_cfg_file *p_cfg)
{
	char temp[1024] = { 0 };

	if ((NULL == db_name) || ('\0' == db_name[0]) || (NULL == p_cfg)) {
		return -1;
	}

	if (NULL != s_ldb) {
		return 0;
	}

	time(&LDB_START_TIME);

	G_PAGE_SIZE = getpagesize();

	LDB_READONLY_SWITCH = p_cfg->ldb_readonly_switch;

	snprintf(temp, sizeof(temp), "%s/%s", p_cfg->work_path, db_name);

	s_ldb = ldb_initialize(temp, p_cfg->ldb_block_size, p_cfg->ldb_write_buffer_size, p_cfg->ldb_cache_lru_size, p_cfg->ldb_bloom_key_size, p_cfg->ldb_compaction_speed);

	if (NULL == s_ldb) {
		x_printf(F, "ldb_initialize failed!");
		return -1;
	}

	/* initial binlog. */
	binlog_open(s_ldb, p_cfg->has_slave);

	/* initial slave. */
	if (slave_open(s_ldb, p_cfg->has_slave, p_cfg->slave_ip, p_cfg->slave_wport) != 0) {
		binlog_close();
		ldb_close(s_ldb);
		s_ldb = NULL;
		return -1;
	}

	return 0;
}

int tsdb_ldb_close(void)
{
	x_printf(I, "close ldb.");

	slave_close();

	binlog_close();

	ldb_close(s_ldb);
	s_ldb = NULL;

	return 0;
}

int tsdb_ldb_set(struct data_node *p_node)
{
	int                     ok = 0;
	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	if ((LDB_READONLY_SWITCH == 1) || (p_rst->keys != 1) || (p_rst->vals != 1)) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	ok = binlog_put(s_ldb, p_buf + p_rst->key_offset[0], p_rst->klen_array[0], p_buf + p_rst->val_offset[0], p_rst->vlen_array[0]);

	if ((ok == 0) || (ok == 1)) {
		cache_add(&p_node->send, OPT_OK, strlen(OPT_OK));
		return X_DONE_OK;
	} else {
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	return X_DONE_OK;
}

int tsdb_ldb_del(struct data_node *p_node)
{
	char                    ret_num[16] = { 0 };
	int                     i = 0;
	int                     ok = 0;
	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	if ((LDB_READONLY_SWITCH == 1) || (p_rst->keys < 1)) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	for (i = 0; i < p_rst->keys; ++i) {
		ok = binlog_batch_delete(s_ldb, p_buf + p_rst->key_offset[i], p_rst->klen_array[i]);

		if (ok < 0) {
			cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	ok = binlog_batch_commit(s_ldb);

	if (ok < 0) {
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	sprintf(ret_num, OPT_NUMBER, p_rst->keys);
	cache_add(&p_node->send, ret_num, strlen(ret_num));

	return X_DONE_OK;
}

int tsdb_ldb_mset(struct data_node *p_node)
{
	int                     i = 0;
	int                     ok = 0;
	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	if ((LDB_READONLY_SWITCH == 1) || (p_rst->keys < 1) || (p_rst->keys != p_rst->vals)) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	for (i = 0; i < p_rst->keys; ++i) {
		ok = binlog_batch_put(s_ldb, p_buf + p_rst->key_offset[i], p_rst->klen_array[i], p_buf + p_rst->val_offset[i], p_rst->vlen_array[i]);

		if (ok < 0) {
			cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	ok = binlog_batch_commit(s_ldb);

	if (ok < 0) {
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	cache_add(&p_node->send, OPT_OK, strlen(OPT_OK));

	return X_DONE_OK;
}

int tsdb_ldb_get(struct data_node *p_node)
{
	char                    *result = NULL;
	int                     size = 0;
	int                     ret = 0;
	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	result = ldb_get(s_ldb, p_buf + p_rst->key_offset[0], p_rst->klen_array[0], &size);

	if (NULL != result) {
		char tmp[32] = { 0 };

		if (size == 0) {
			free(result);
			cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}

		sprintf(tmp, "$%d\r\n", size);
		ret = cache_add(&p_node->send, tmp, strlen(tmp));
		ret = cache_add(&p_node->send, result, size);
		free(result);

		if (ret == X_MALLOC_FAILED) {
			cache_free(&p_node->send);
			cache_add(&p_node->send, OPT_NO_MEMORY, strlen(OPT_NO_MEMORY));
			return X_MALLOC_FAILED;
		}

		ret = cache_add(&p_node->send, "\r\n", 2);
		return X_DONE_OK;
	} else {
		if (size == 0) {
			cache_add(&p_node->send, OPT_NULL, strlen(OPT_NULL));
			return X_DONE_OK;
		} else {
			cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	return X_DONE_OK;
}

int tsdb_ldb_lrange(struct data_node *p_node)
{
	char                    *result = NULL;
	int                     size = 0;
	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	if ((p_rst->keys != 1) || (p_rst->vals != 2)) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	result = ldb_lrangeget(s_ldb, p_buf + p_rst->key_offset[0], p_rst->klen_array[0], p_buf + p_rst->val_offset[0], p_rst->vlen_array[0],
			p_buf + p_rst->val_offset[1], p_rst->vlen_array[1], &size);

	if (result) {
		cache_set(&p_node->send, result, size, size);
		return X_DONE_OK;
	} else {
		if (size == 0) {
			cache_add(&p_node->send, OPT_EMPTY_MULTI, strlen(OPT_EMPTY_MULTI));
			return X_DONE_OK;
		} else {
			cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	return X_DONE_OK;
}

int tsdb_ldb_keys(struct data_node *p_node)
{
	char                    *result = NULL;
	int                     size = 0;
	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	if (p_rst->keys != 1) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	result = ldb_keys(s_ldb, p_buf + p_rst->key_offset[0], p_rst->klen_array[0], &size);

	if (result) {
		cache_set(&p_node->send, result, size, size);
		return X_DONE_OK;
	} else {
		if (size == 0) {
			cache_add(&p_node->send, OPT_EMPTY_MULTI, strlen(OPT_EMPTY_MULTI));
			return X_DONE_OK;
		} else {
			cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	return X_DONE_OK;
}

int tsdb_ldb_values(struct data_node *p_node)
{
	char                    *result = NULL;
	int                     size = 0;
	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	if (p_rst->keys != 1) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	result = ldb_values(s_ldb, p_buf + p_rst->key_offset[0], p_rst->klen_array[0], &size);

	if (result) {
		cache_set(&p_node->send, result, size, size);
		return X_DONE_OK;
	} else {
		if (size == 0) {
			cache_add(&p_node->send, OPT_EMPTY_MULTI, strlen(OPT_EMPTY_MULTI));
			return X_DONE_OK;
		} else {
			cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	return X_DONE_OK;
}

int tsdb_ldb_info(struct data_node *p_node)
{
	char    *result = NULL;
	int     size = 0;

	result = ldb_info(s_ldb, &size);

	if (result) {
		cache_set(&p_node->send, result, size, size);
		return X_DONE_OK;
	} else {
		if (size == 0) {
			cache_add(&p_node->send, OPT_EMPTY_MULTI, strlen(OPT_EMPTY_MULTI));
			return X_DONE_OK;
		} else {
			cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
			return X_EXECUTE_ERROR;
		}
	}

	return X_DONE_OK;
}

int tsdb_ldb_ping(struct data_node *p_node)
{
	cache_add(&p_node->send, OPT_PONG, strlen(OPT_PONG));

	return 0;
}

int tsdb_ldb_exists(struct data_node *p_node)
{
	int                     ok = 0;
	char                    ret_num[16] = { 0 };
	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	ok = ldb_exists(s_ldb, p_buf + p_rst->key_offset[0], p_rst->klen_array[0]);

	sprintf(ret_num, OPT_NUMBER, ok);
	cache_add(&p_node->send, ret_num, strlen(ret_num));

	return 0;
}

int tsdb_ldb_syncset(struct data_node *p_node)
{
	int                     ok = 0;
	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	if ((p_rst->keys != 1) || (p_rst->vals != 1)) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	ok = binlog_syncset(s_ldb, p_buf + p_rst->key_offset[0], p_rst->klen_array[0], p_buf + p_rst->val_offset[0], p_rst->vlen_array[0]);

	if ((ok == 0) || (ok == 1)) {
		cache_add(&p_node->send, OPT_OK, strlen(OPT_OK));
		return X_DONE_OK;
	} else {
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	return X_DONE_OK;
}

int tsdb_ldb_syncdel(struct data_node *p_node)
{
	int                     ok = 0;
	char                    *p_buf = p_node->recv.buf_addr;
	struct redis_status     *p_rst = &p_node->redis_info.rs;

	if (p_rst->keys < 1) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	ok = binlog_syncdel(s_ldb, p_buf + p_rst->key_offset[0], p_rst->klen_array[0]);

	if ((ok == 0) || (ok == 1)) {
		cache_add(&p_node->send, OPT_OK, strlen(OPT_OK));
		return X_DONE_OK;
	} else {
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	return X_DONE_OK;
}

int tsdb_ldb_compact(struct data_node *p_node)
{
	ldb_compact(s_ldb);

	cache_add(&p_node->send, OPT_OK, strlen(OPT_OK));

	return X_DONE_OK;
}

