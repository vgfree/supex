#include "utils.h"
#include "tsdb_cfg.h"
#include "tsdb_ldb.h"
#include "tsdb_kv.h"

#define OPT_PONG "+PONG\r\n"

#define IS_NULL(x) (((x)[0] == '$') && ((x)[1] == '-') && ((x)[2] == '1') && ((x)[3] == '\r') && ((x)[4] == '\n'))

static TSDB_ENGINE_e s_engine_type = TSDB_ENGINE_UNKNOWN;

int tsdb_cmd_init(struct tsdb_cfg_file *p_cfg)
{
	int ret = 0;

	if ((p_cfg->engine_type < TSDB_ENGINE_LDB) || (p_cfg->engine_type > TSDB_ENGINE_MIX)) {
		x_printf(E, "engine_type = %d,err", (int)p_cfg->engine_type);
		return -1;
	}

	s_engine_type = p_cfg->engine_type;

	if (s_engine_type & TSDB_ENGINE_LDB) {
		ret = tsdb_ldb_init(LDB_DB_NAME, p_cfg);
	}

	if (s_engine_type & TSDB_ENGINE_KV) {
		ret = tsdb_kv_init();
	}

	return 0;
}

int tsdb_cmd_close(void)
{
	if (s_engine_type & TSDB_ENGINE_LDB) {
		tsdb_ldb_close();
	}

	if (s_engine_type & TSDB_ENGINE_KV) {
		tsdb_kv_close();
	}

	return 0;
}

int tsdb_cmd_set(struct data_node *p_node)
{
	switch (s_engine_type)
	{
		case TSDB_ENGINE_LDB:
			return tsdb_ldb_set(p_node);

			break;

		case TSDB_ENGINE_KV:
			return tsdb_kv_set(p_node);

			break;

		case TSDB_ENGINE_MIX:
			tsdb_kv_set(p_node);
			cache_clean(&p_node->mdl_send.cache);	// FIXME
			return tsdb_ldb_set(p_node);

			break;

		default:
			break;
	}

	cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
	return X_INTERIOR_ERROR;
}

int tsdb_cmd_del(struct data_node *p_node)
{
	switch (s_engine_type)
	{
		case TSDB_ENGINE_LDB:
			return tsdb_ldb_del(p_node);

			break;

		case TSDB_ENGINE_KV:
			return tsdb_kv_del(p_node);

			break;

		case TSDB_ENGINE_MIX:
			tsdb_kv_del(p_node);
			cache_clean(&p_node->mdl_send.cache);
			return tsdb_ldb_del(p_node);

			break;

		default:
			break;
	}

	cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
	return X_INTERIOR_ERROR;
}

int tsdb_cmd_mset(struct data_node *p_node)
{
	switch (s_engine_type)
	{
		case TSDB_ENGINE_LDB:
			return tsdb_ldb_mset(p_node);

			break;

		case TSDB_ENGINE_KV:
			return tsdb_kv_mset(p_node);

			break;

		case TSDB_ENGINE_MIX:
			tsdb_kv_mset(p_node);
			cache_clean(&p_node->mdl_send.cache);
			return tsdb_ldb_mset(p_node);

			break;

		default:
			break;
	}

	cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
	return X_INTERIOR_ERROR;
}

int tsdb_cmd_sadd(struct data_node *p_node)
{
	switch (s_engine_type)
        {
                case TSDB_ENGINE_LDB:
                        return tsdb_ldb_sadd(p_node);

                        break;

                case TSDB_ENGINE_KV:
                        //return tsdb_kv_set(p_node);
                        printf("TSDB_ENGINE_KV did't support yet.");
			return X_INTERIOR_ERROR;
                        break;

                case TSDB_ENGINE_MIX:
                        //tsdb_kv_set(p_node);
                        //cache_clean(&p_node->mdl_send.cache);   // FIXME
                        //return tsdb_ldb_set(p_node);
                        printf("TSDB_ENGINE_MIX did't support yet.");
			return X_INTERIOR_ERROR;
                        break;

                default:
                        break;
        }

        cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
        return X_INTERIOR_ERROR;
}

int tsdb_cmd_get(struct data_node *p_node)
{
	switch (s_engine_type)
	{
		case TSDB_ENGINE_LDB:
			return tsdb_ldb_get(p_node);

			break;

		case TSDB_ENGINE_KV:
			return tsdb_kv_get(p_node);

			break;

		case TSDB_ENGINE_MIX:

			// FIXME
			if ((tsdb_kv_get(p_node) == X_DONE_OK) && (!IS_NULL(p_node->mdl_send.cache.buff))) {
				return X_DONE_OK;
			} else {
				cache_clean(&p_node->mdl_send.cache);
				return tsdb_ldb_get(p_node);
			}

			break;

		default:
			break;
	}

	cache_append(&p_node->mdl_send.cache, OPT_INTERIOR_ERROR, strlen(OPT_INTERIOR_ERROR));
	return X_INTERIOR_ERROR;
}

int tsdb_cmd_lrange(struct data_node *p_node)
{
	if (s_engine_type & TSDB_ENGINE_LDB) {
		return tsdb_ldb_lrange(p_node);
	}

	cache_append(&p_node->mdl_send.cache, OPT_NO_THIS_CMD, strlen(OPT_NO_THIS_CMD));
	return X_INTERIOR_ERROR;
}

int tsdb_cmd_keys(struct data_node *p_node)
{
	if (s_engine_type & TSDB_ENGINE_LDB) {
		return tsdb_ldb_keys(p_node);
	}

	cache_append(&p_node->mdl_send.cache, OPT_NO_THIS_CMD, strlen(OPT_NO_THIS_CMD));
	return X_INTERIOR_ERROR;
}

int tsdb_cmd_values(struct data_node *p_node)
{
	if (s_engine_type & TSDB_ENGINE_LDB) {
		return tsdb_ldb_values(p_node);
	}

	cache_append(&p_node->mdl_send.cache, OPT_NO_THIS_CMD, strlen(OPT_NO_THIS_CMD));
	return X_INTERIOR_ERROR;
}

int tsdb_cmd_info(struct data_node *p_node)
{
	if (s_engine_type & TSDB_ENGINE_LDB) {
		return tsdb_ldb_info(p_node);
	}

	cache_append(&p_node->mdl_send.cache, OPT_NO_THIS_CMD, strlen(OPT_NO_THIS_CMD));
	return X_INTERIOR_ERROR;
}

int tsdb_cmd_ping(struct data_node *p_node)
{
	cache_append(&p_node->mdl_send.cache, OPT_PONG, strlen(OPT_PONG));
	return X_DONE_OK;
}

int tsdb_cmd_exists(struct data_node *p_node)
{
	if (s_engine_type & TSDB_ENGINE_LDB) {
		return tsdb_ldb_exists(p_node);
	}

	cache_append(&p_node->mdl_send.cache, OPT_NO_THIS_CMD, strlen(OPT_NO_THIS_CMD));
	return X_INTERIOR_ERROR;
}

int tsdb_cmd_syncset(struct data_node *p_node)
{
	if (s_engine_type & TSDB_ENGINE_LDB) {
		return tsdb_ldb_syncset(p_node);
	}

	cache_append(&p_node->mdl_send.cache, OPT_NO_THIS_CMD, strlen(OPT_NO_THIS_CMD));
	return X_INTERIOR_ERROR;
}

int tsdb_cmd_syncdel(struct data_node *p_node)
{
	if (s_engine_type & TSDB_ENGINE_LDB) {
		return tsdb_ldb_syncdel(p_node);
	}

	cache_append(&p_node->mdl_send.cache, OPT_NO_THIS_CMD, strlen(OPT_NO_THIS_CMD));
	return X_INTERIOR_ERROR;
}

int tsdb_cmd_compact(struct data_node *p_node)
{
	if (s_engine_type & TSDB_ENGINE_LDB) {
		return tsdb_ldb_compact(p_node);
	}

	cache_append(&p_node->mdl_send.cache, OPT_NO_THIS_CMD, strlen(OPT_NO_THIS_CMD));
	return X_INTERIOR_ERROR;
}

