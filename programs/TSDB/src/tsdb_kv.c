#include "libkv.h"

#include "utils.h"
#include "tsdb_kv.h"


#define OPT_NULL                "$-1\r\n"

static int s_kv = -1;

int tsdb_kv_init(char *ident)
{
	if (-1 != s_kv) {
		return 0;
	}

	kv_init();

	struct kv_config cfg = {
		.open_on_disk   = false,
		.time_to_stay   = -1,
	};
	s_kv = kv_load(&cfg, ident);
	if (-1 == s_kv) {
		x_printf(F, "kv_load failed");
		return -1;
	}

	return 0;
}

int tsdb_kv_close(void)
{
	if (-1 != s_kv) {
		kv_destroy();
		s_kv = -1;
	}

	return 0;
}

int tsdb_kv_set(struct data_node *p_node)
{
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	unsigned                u_size = cache_data_length(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	assert(s_kv == 0);

	if (p_rst->fields != 2) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	/* SET */
	kv_handler_t *handler = kv_opt(s_kv, "set", p_buf + p_rst->field[0].offset, p_rst->field[0].len,
		p_buf + p_rst->field[1].offset, p_rst->field[1].len);

	kv_answer_t *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	unsigned long count = answer_length(ans);
	if (count != 1) {
		kv_handler_release(handler);
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

        kv_answer_value_t *value = answer_head_value(ans);
	char *data = (char *)answer_value_look_addr(value);
	size_t size = answer_value_look_size(value);

	if (strncasecmp("OK", (char *)data, size) != 0) {
		kv_handler_release(handler);
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	kv_handler_release(handler);
	cache_append(&p_node->mdl_send.cache, OPT_OK, strlen(OPT_OK));
	return X_DONE_OK;
}

int tsdb_kv_del(struct data_node *p_node)
{
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	unsigned                u_size = cache_data_length(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if (p_rst->fields < 1) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	assert(s_kv == 0);

	struct kv_argv args[p_rst->fields];
	int i = 0;
	for (i=0; i < p_rst->fields; i++) {
		args[i].ptr = p_buf + p_rst->field[i].offset;
		args[i].len = p_rst->field[i].len;
	}
	kv_handler_t *handler = kv_arg(s_kv, "del", p_rst->fields, args);

	kv_answer_t *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	unsigned long count = answer_length(ans);
	if (count != 1) {
		kv_handler_release(handler);
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

        kv_answer_value_t *value = answer_head_value(ans);
	assert(answer_value_look_type(value) == VALUE_TYPE_INT);

	char                    result[32] = { 0 };
	sprintf(result, ":%d\r\n", answer_value_look_int(value));
	cache_append(&p_node->mdl_send.cache, result, strlen(result));

	kv_handler_release(handler);

	return X_DONE_OK;
}

int tsdb_kv_mset(struct data_node *p_node)
{
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	unsigned                u_size = cache_data_length(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;

	if ((p_rst->fields < 1) || (p_rst->fields % 2 != 0)) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	assert(s_kv == 0);

	/* MSET */
	struct kv_argv args[p_rst->fields];
	int i = 0;
	for (i=0; i < p_rst->fields; i++) {
		args[i].ptr = p_buf + p_rst->field[i].offset;
		args[i].len = p_rst->field[i].len;
	}
	kv_handler_t *handler = kv_arg(s_kv, "mset", p_rst->fields, args);

	kv_answer_t *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	unsigned long count = answer_length(ans);
	if (count != 1) {
		kv_handler_release(handler);
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

        kv_answer_value_t *value = answer_head_value(ans);
	char *data = (char *)answer_value_look_addr(value);
	size_t size = answer_value_look_size(value);

	if (strncasecmp("OK", (char *)data, size) != 0) {
		kv_handler_release(handler);
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	kv_handler_release(handler);
	cache_append(&p_node->mdl_send.cache, OPT_OK, strlen(OPT_OK));
	return X_DONE_OK;
}

int tsdb_kv_sadd(struct data_node *p_node)
{
	//TODO
	return 1;
}

int tsdb_kv_get(struct data_node *p_node)
{
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	unsigned                u_size = cache_data_length(&p_node->mdl_recv.cache);
	struct redis_status     *p_rst = &p_node->mdl_recv.parse.redis_info.rs;
	int                     ret = 0;

	if (p_rst->fields < 1) {
		cache_append(&p_node->mdl_send.cache, OPT_CMD_ERROR, strlen(OPT_CMD_ERROR));
		return X_EXECUTE_ERROR;
	}

	assert(s_kv == 0);

	kv_handler_t *handler = kv_opt(s_kv, "get", p_buf + p_rst->field[0].offset, p_rst->field[0].len);

	kv_answer_t *ans = &handler->answer;

	if (ERR_NONE != ans->errnum) {
		x_printf(E, "errnum:%d\terr:%s\n\n", ans->errnum, error_getinfo(ans->errnum));
		kv_handler_release(handler);
		
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

	unsigned long count = answer_length(ans);
	if (count != 1) {
		kv_handler_release(handler);
		cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
		return X_EXECUTE_ERROR;
	}

        kv_answer_value_t *value = answer_head_value(ans);

	if (answer_value_look_type(value) == VALUE_TYPE_NIL) {
		cache_append(&p_node->mdl_send.cache, OPT_NULL, strlen(OPT_NULL));
	} else {
		char *data = (char *)answer_value_look_addr(value);
		size_t size = answer_value_look_size(value);
		char                    tmp[32] = { 0 };
		sprintf(tmp, "$%d\r\n", size);
		ret = cache_append(&p_node->mdl_send.cache, tmp, strlen(tmp));
		ret = cache_append(&p_node->mdl_send.cache, data, size);
		if (ret == X_MALLOC_FAILED) {
			cache_clean(&p_node->mdl_send.cache);
			cache_append(&p_node->mdl_send.cache, OPT_NO_MEMORY, strlen(OPT_NO_MEMORY));
			kv_handler_release(handler);
			return X_MALLOC_FAILED;
		}
		ret = cache_append(&p_node->mdl_send.cache, "\r\n", 2);
	}


	kv_handler_release(handler);
	return X_DONE_OK;
}
