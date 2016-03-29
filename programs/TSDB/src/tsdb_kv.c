#include "kv_priv.h"

#include "utils.h"
#include "tsdb_kv.h"

#define DEFAULT_EXPIRE_TIME     (2 * 3600)

#define OPT_NULL                "$-1\r\n"

static kv_handler_t *s_kv = NULL;

int tsdb_kv_init(void)
{
	if (NULL != s_kv) {
		return 0;
	}

	s_kv = kv_create(NULL);

	if (NULL == s_kv) {
		x_printf(F, "kv_create failed");
		return -1;
	}

	return 0;
}

int tsdb_kv_close(void)
{
	if (NULL != s_kv) {
		kv_destroy(s_kv);
		s_kv = NULL;
	}

	return 0;
}

int tsdb_kv_set(struct data_node *p_node)
{
	struct redis_status     *p_rst = &p_node->redis_info.rs;
	kv_answer_t             *ans = NULL;
	kv_answer_value_t       *value = 0;
	unsigned long           count = 0;
	int                     len = 0;
	char                    cmd[256] = { 0 };

	assert(s_kv);

	if ((p_rst->keys != 1) || (p_rst->vals != 1)) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	/* SET */
	ans = kv_ask_proto(s_kv, p_node->recv.buf_addr, (unsigned int)p_node->recv.get_size);

	if ((NULL == ans) || (ERR_NONE != ans->errnum)) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	count = kv_answer_length(ans);

	if (count != 1) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	value = kv_answer_first_value(ans);

	if ((NULL == value) || (NULL == value->ptr)) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	if (strncasecmp("OK", (char *)(value->ptr), value->ptrlen) != 0) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	kv_answer_release(ans);

#ifdef OPEN_KV_EXPIRE
	/* EXPIRE , err ingnore */
	len += sprintf(cmd, "EXPIRE ");
	memcpy(cmd + len, p_node->recv.buf_addr + p_rst->key_offset[0], p_rst->klen_array[0]);
	len += (int)p_rst->klen_array[0];
	len += sprintf(cmd + len, " %d", DEFAULT_EXPIRE_TIME);

	ans = kv_ask(s_kv, cmd, len);

	if ((NULL != ans) && (ERR_NONE == ans->errnum)) {
		count = kv_answer_length(ans);

		if (count == 1) {
			value = kv_answer_first_value(ans);

			if ((NULL == value) || (NULL == value->ptr) || (((char *)value->ptr)[0] != '1')) {
				x_printf(W, "cmd: %s err!", cmd);
			}
		} else {
			x_printf(W, "cmd: %s err!", cmd);
		}
	} else {
		x_printf(W, "cmd: %s err!", cmd);
	}
	kv_answer_release(ans);
#endif	/* ifdef OPEN_KV_EXPIRE */

	cache_add(&p_node->send, OPT_OK, strlen(OPT_OK));
	return X_DONE_OK;
}

int tsdb_kv_del(struct data_node *p_node)
{
	struct redis_status     *p_rst = &p_node->redis_info.rs;
	kv_answer_t             *ans = NULL;
	kv_answer_value_t       *value = 0;
	unsigned long           count = 0;
	char                    result[32] = { 0 };

	if (p_rst->keys < 1) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	assert(s_kv);

	ans = kv_ask_proto(s_kv, p_node->recv.buf_addr, (unsigned int)p_node->recv.get_size);

	if ((NULL == ans) || (ERR_NONE != ans->errnum)) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	if (ERR_NONE != ans->errnum) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	count = kv_answer_length(ans);

	if (count != 1) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	value = kv_answer_first_value(ans);

	if ((NULL == value) || (NULL == value->ptr)) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	sprintf(result, ":%s\r\n", ((char *)value->ptr));
	cache_add(&p_node->send, result, strlen(result));

	kv_answer_release(ans);

	return X_DONE_OK;
}

int tsdb_kv_mset(struct data_node *p_node)
{
	struct redis_status     *p_rst = &p_node->redis_info.rs;
	kv_answer_t             *ans = NULL;
	kv_answer_value_t       *value = 0;
	unsigned long           count = 0;
	int                     len = 0;
	char                    cmd[256] = { 0 };
	int                     i = 0;

	if ((p_rst->keys < 1) || (p_rst->keys != p_rst->vals)) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	assert(s_kv);

	/* MSET */
	ans = kv_ask_proto(s_kv, p_node->recv.buf_addr, (unsigned int)p_node->recv.get_size);

	if ((NULL == ans) || (ERR_NONE != ans->errnum)) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	if (ERR_NONE != ans->errnum) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	count = kv_answer_length(ans);

	if (count != 1) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	value = kv_answer_first_value(ans);

	if ((NULL == value) || (NULL == value->ptr)) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	if (strncasecmp("OK", ((char *)value->ptr), value->ptrlen) != 0) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	kv_answer_release(ans);

#ifdef OPEN_KV_EXPIRE
	/* EXPIRE , err ingnore */
	for (i = 0; i < p_rst->keys; ++i) {
		len = sprintf(cmd, "EXPIRE ");
		memcpy(cmd + len, p_node->recv.buf_addr + p_rst->key_offset[i], p_rst->klen_array[i]);
		len += (int)p_rst->klen_array[0];
		len += sprintf(cmd + len, " %d", DEFAULT_EXPIRE_TIME);
		cmd[len] = '\0';

		ans = kv_ask(s_kv, cmd, len);

		if ((NULL != ans) && (ERR_NONE == ans->errnum)) {
			count = kv_answer_length(ans);

			if (count == 1) {
				value = kv_answer_first_value(ans);

				if ((NULL == value) || (NULL == value->ptr) || (((char *)value->ptr)[0] != '1')) {
					x_printf(W, "cmd: %s err!", cmd);
				}
			} else {
				x_printf(W, "cmd: %s err!", cmd);
			}
		} else {
			x_printf(W, "cmd: %s err!", cmd);
		}

		kv_answer_release(ans);
	}
#endif	/* ifdef OPEN_KV_EXPIRE */

	cache_add(&p_node->send, OPT_OK, strlen(OPT_OK));

	return X_DONE_OK;
}

int tsdb_kv_get(struct data_node *p_node)
{
	struct redis_status     *p_rst = &p_node->redis_info.rs;
	kv_answer_t             *ans = NULL;
	kv_answer_value_t       *value = 0;
	unsigned long           count = 0;
	char                    tmp[32] = { 0 };
	int                     ret = 0;

	if (p_rst->keys < 1) {
		cache_add(&p_node->send, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	assert(s_kv);

	ans = kv_ask_proto(s_kv, p_node->recv.buf_addr, (unsigned int)p_node->recv.get_size);

	if (NULL == ans) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	if (ERR_NIL == ans->errnum) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_NULL, strlen(OPT_NULL));
		return X_DONE_OK;
	} else if (ERR_NONE != ans->errnum) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	count = kv_answer_length(ans);

	if (count != 1) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	value = kv_answer_first_value(ans);

	if ((NULL == value) || (NULL == value->ptr) || (0 == value->ptrlen)) {
		kv_answer_release(ans);
		cache_add(&p_node->send, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	sprintf(tmp, "$%d\r\n", (int)value->ptrlen);
	ret = cache_add(&p_node->send, tmp, strlen(tmp));
	ret = cache_add(&p_node->send, ((char *)value->ptr), (int)value->ptrlen);

	if (ret == X_MALLOC_FAILED) {
		cache_free(&p_node->send);
		cache_add(&p_node->send, OPT_NO_MEMORY, strlen(OPT_NO_MEMORY));
		kv_answer_release(ans);
		return X_MALLOC_FAILED;
	}

	ret = cache_add(&p_node->send, "\r\n", 2);

	kv_answer_release(ans);

	return X_DONE_OK;
}

#if 0
int main(void)
{
	kv_answer_t *ans = NULL;

	tsdb_kv_init();

	ans = kv_ask_proto(s_kv, "*3\r\n$3\r\nSET\r\n$1\r\na\r\n$1\r\n\xff\r\n", strlen("*3\r\n$3\r\nSET\r\n$1\r\na\r\n$1\r\n\r\n") + 1);
	ans = kv_ask_proto(s_kv, "*2\r\n$3\r\nGET\r\n$1\r\nb\r\n", strlen("*2\r\n$3\r\nGET\r\n$1\r\nb\r\n"));

	tsdb_kv_close();

	return 0;
}
#endif

