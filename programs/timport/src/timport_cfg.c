#include "json.h"

#include "utils.h"

#include "timport_cfg.h"
#include "timport_filter.h"

#define MIN_DELAY_TIME  3600
#define MIN_EXPIRE_TIME 1800

#define loger(fmt, ...) fprintf(stderr, "(%s:%d) " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

static tkey_type_e tkey_type_lookup(const char *str)
{
	tkey_type_e type = TKEY_TYPE_UNKNOWN;

	if (NULL == str) {
		return type;
	}

	if (0 == strcasecmp(str, "WHOLE_INDEX")) {
		type = TKEY_TYPE_WHOLE_INDEX;
	} else if (0 == strcasecmp(str, "ALONE_INDEX")) {
		type = TKEY_TYPE_ALONE_INDEX;
	} else if (0 == strcasecmp(str, "KEYS_INDEX")) {
		type = TKEY_TYPE_KEYS_INDEX;
	} else if (0 == strcasecmp(str, "SET_VAL")) {
		type = TKEY_TYPE_SET_VAL;
	} else if (0 == strcasecmp(str, "STRING_VAL")) {
		type = TKEY_TYPE_STRING_VAL;
	}

	return type;
}

static tkey_interval_e tkey_interval_lookup(const char *str)
{
	tkey_interval_e type = TKEY_INTERVAL_UNKNOWN;

	if (NULL == str) {
		return type;
	}

	if (0 == strcasecmp(str, "TEN_MIN")) {
		type = TKEY_INTERVAL_TEN_MIN;
	} else if (0 == strcasecmp(str, "ONE_HOUR")) {
		type = TKEY_INTERVAL_ONE_HOUR;
	}

	return type;
}

static int load_tkey_from_jso(timport_key_t *p_tkey, struct json_object *jso, struct timport_cfg_file *p_cfg)
{
	int                     i = 0;
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;

	if ((NULL == p_tkey) || (NULL == jso)) {
		return -1;
	}

	if (json_object_object_get_ex(jso, "key", &obj)) {
		str_val = json_object_get_string(obj);
		p_tkey->key = x_strdup(str_val);
	} else {
		loger("(key) not found");
		goto fail;
	}

	if (json_object_object_get_ex(jso, "type", &obj)) {
		str_val = json_object_get_string(obj);
		p_tkey->type = tkey_type_lookup(str_val);

		if (TKEY_TYPE_UNKNOWN == p_tkey->type) {
			loger("(type) must be \"WHOLE_INDEX\" or \"ALONE_INDEX\" or \"KEYS_INDEX\" or \"SET_VAL\" or \"STRING_VAL\", under key (%s)", p_tkey->key);
			goto fail;
		}
	} else {
		loger("(type) not found, under key (%s)", p_tkey->key);
		goto fail;
	}

	if (json_object_object_get_ex(jso, "interval", &obj)) {
		str_val = json_object_get_string(obj);
		p_tkey->interval = tkey_interval_lookup(str_val);

		if (TKEY_INTERVAL_UNKNOWN == p_tkey->interval) {
			if ((TKEY_TYPE_WHOLE_INDEX == p_tkey->type) || (TKEY_TYPE_KEYS_INDEX == p_tkey->type)) {
				loger("(interval) must be \"TEN_MIN\" or \"ONE_HOUR\"");
				goto fail;
			}
		} else if (TKEY_INTERVAL_TEN_MIN == p_tkey->interval) {
			p_cfg->has_ten_min = 1;
		}
	} else {
		if ((TKEY_TYPE_WHOLE_INDEX == p_tkey->type) || (TKEY_TYPE_KEYS_INDEX == p_tkey->type)) {
			loger("(interval) not found, under key (%s)", p_tkey->key);
			goto fail;
		}
	}

	if (json_object_object_get_ex(jso, "expire_time", &obj)) {
		p_tkey->expire_time = json_object_get_int(obj);

		if (p_tkey->expire_time < MIN_EXPIRE_TIME) {
			loger("(expire_time) less than %d, under key (%s)", MIN_EXPIRE_TIME, p_tkey->key);
			goto fail;
		}
	} else {
		if ((TKEY_TYPE_WHOLE_INDEX == p_tkey->type) || (TKEY_TYPE_KEYS_INDEX == p_tkey->type)) {
			loger("(expire_time) not found, under key (%s)", p_tkey->key);
			goto fail;
		} else {
			p_tkey->expire_time = p_tkey->parent->expire_time;
		}
	}

	if (json_object_object_get_ex(jso, "param_cnt", &obj)) {
		p_tkey->param_cnt = json_object_get_int(obj);

		if ((1 != p_tkey->param_cnt) && (2 != p_tkey->param_cnt)) {
			loger("(param_cnt) must be 1 or 2, under key (%s)", p_tkey->key);
			goto fail;
		}
	} else {
		if ((TKEY_TYPE_WHOLE_INDEX == p_tkey->type) || (TKEY_TYPE_KEYS_INDEX == p_tkey->type)) {
			p_tkey->param_cnt = 1;
		} else {
			loger("(param_cnt) not found, under key (%s)", p_tkey->key);
			goto fail;
		}
	}

	if (json_object_object_get_ex(jso, "param_tm_pos", &obj)) {
		p_tkey->param_tm_pos = json_object_get_int(obj);

		if ((0 != p_tkey->param_tm_pos) && (1 != p_tkey->param_tm_pos)) {
			loger("(param_tm_pos) must be 0 or 1, under key (%s)", p_tkey->key);
			goto fail;
		}
	} else {
		if (p_tkey->param_cnt == 1) {
			p_tkey->param_tm_pos = 0;
		} else {
			loger("(param_tm_pos) not found, under key (%s)", p_tkey->key);
			goto fail;
		}
	}

	if (json_object_object_get_ex(jso, "redis_filter", &obj)) {
		str_val = json_object_get_string(obj);
		p_tkey->redis_filter = (TKEY_REDIS_FILTER)timport_filter_lookup(str_val);

		if (NULL == p_tkey->redis_filter) {
			loger("(redis_filter) \"%s\" not found, under key (%s)", str_val, p_tkey->key);
			goto fail;
		}
	}

	if (json_object_object_get_ex(jso, "result_filter", &obj)) {
		str_val = json_object_get_string(obj);
		p_tkey->result_filter = (TKEY_RESULT_FILTER)timport_filter_lookup(str_val);

		if (NULL == p_tkey->result_filter) {
			loger("(result_filter) \"%s\" not found, under key (%s)", str_val, p_tkey->key);
			goto fail;
		}
	}

	if (json_object_object_get_ex(jso, "key_filter", &obj)) {
		str_val = json_object_get_string(obj);
		p_tkey->key_filter = (TKEY_KEY_FILTER)timport_filter_lookup(str_val);

		if (NULL == p_tkey->key_filter) {
			loger("(key_filter) \"%s\" not found, under key (%s)", str_val, p_tkey->key);
			goto fail;
		}
	}

	if (json_object_object_get_ex(jso, "hash_filter", &obj)) {
		str_val = json_object_get_string(obj);
		p_tkey->hash_filter = (TKEY_HASH_FILTER)timport_filter_lookup(str_val);

		if (NULL == p_tkey->hash_filter) {
			loger("(hash_filter) \"%s\" not found, under key (%s)", str_val, p_tkey->key);
			goto fail;
		}
	} else {
		if ((TKEY_TYPE_WHOLE_INDEX != p_tkey->type) && (TKEY_TYPE_KEYS_INDEX != p_tkey->type)) {
			loger("(hash_filter) not found, under key (%s)", p_tkey->key);
			goto fail;
		}
	}

	if (p_cfg->zk_disabled) {
		p_tkey->tsdb = &p_cfg->tsdb;
	} else {
		if (json_object_object_get_ex(jso, "tsdb_in_cfg", &obj)) {
			if (json_object_get_int(obj)) {
				if (p_cfg->tsdb.kset_cnt <= 0) {
					loger("(tsdb_in_cfg) need (tsdb), under key (%s)", p_tkey->key);
					goto fail;
				}

				p_tkey->tsdb = &p_cfg->tsdb;
			}
		}
	}

	if (json_object_object_get_ex(jso, "statistics", &obj)) {
		str_val = json_object_get_string(obj);

		if (p_cfg->statistics.keys_cnt == 0) {
			loger("(statistics) needed by key (%s)", p_tkey->key);
			goto fail;
		}

		for (i = 0; i < p_cfg->statistics.keys_cnt; ++i) {
			if (strcasecmp(str_val, p_cfg->statistics.keys[i].name) == 0) {
				p_tkey->stat = &(p_cfg->statistics.keys[i]);
				break;
			}
		}

		if (i == p_cfg->statistics.keys_cnt) {
			loger("(statistics) has no named \"%s\", under key (%s)", str_val, p_tkey->key);
			goto fail;
		}
	}

	if (json_object_object_get_ex(jso, "ignore_error", &obj)) {
		p_tkey->ignore_error = json_object_get_int(obj);
	}

	if ((TKEY_TYPE_WHOLE_INDEX == p_tkey->type) || (TKEY_TYPE_ALONE_INDEX == p_tkey->type) || (TKEY_TYPE_KEYS_INDEX == p_tkey->type)) {
		if (json_object_object_get_ex(jso, "child", &obj)) {
			p_tkey->child_cnt = json_object_array_length(obj);

			if (p_tkey->child_cnt <= 0) {
				loger("(child) have no child, under key (%s)", p_tkey->key);
				goto fail;
			}

			p_tkey->child = calloc(p_tkey->child_cnt, sizeof(timport_key_t));

			if (NULL == p_tkey->child) {
				loger("(child) calloc failed, under key (%s)", p_tkey->key);
				goto fail;
			}

			for (i = 0; i < p_tkey->child_cnt; ++i) {
				p_tkey->child[i].parent = p_tkey;
				int ok = load_tkey_from_jso(&p_tkey->child[i], json_object_array_get_idx(obj, i), p_cfg);

				if (ok != 0) {
					loger("(load_tkey_from_jso) failed, under key (%s)", p_tkey->key);
					goto fail;
				}
			}
		} else {
			loger("(child) not found, under key (%s)", p_tkey->key);
			goto fail;
		}
	}

	return 0;

fail:

	if (NULL != p_tkey->key) {
		free(p_tkey->key);
		p_tkey->key = NULL;
	}

	if (NULL != p_tkey->child) {
		free(p_tkey->child);
		p_tkey->child = NULL;
		p_tkey->child_cnt = 0;
	}

	return -1;
}

static int load_kset_from_jso(tsdb_kset_t *p_kset, struct json_object *jso, struct timport_cfg_file *p_cfg)
{
	int                     i = 0;
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *tmp_obj = NULL;
	struct json_object      *dn_obj = NULL;
	int                     array_len = 0;

	if ((NULL == p_kset) || (NULL == jso)) {
		goto fail;
	}

	if (json_object_object_get_ex(jso, "key_set", &obj)) {
		array_len = json_object_array_length(obj);

		if (2 != array_len) {
			loger("(key_set) array_len != 2");
			goto fail;
		}

		tmp_obj = json_object_array_get_idx(obj, 0);
		p_kset->s_key = (uint64_t)json_object_get_int(tmp_obj);
		tmp_obj = json_object_array_get_idx(obj, 1);
		p_kset->e_key = (uint64_t)json_object_get_int(tmp_obj);
	} else {
		loger("(key_set) not found");
		goto fail;
	}

	if (json_object_object_get_ex(jso, "data_set", &obj)) {
		array_len = json_object_array_length(obj);

		if (array_len > 2) {
			loger("(data_set) array_len > 2");
			goto fail;
		}

		p_kset->dn_cnt = array_len;

		for (i = 0; i < array_len; ++i) {
			tmp_obj = json_object_array_get_idx(obj, i);

			if (json_object_object_get_ex(tmp_obj, "host", &dn_obj)) {
				str_val = json_object_get_string(dn_obj);
				p_kset->data_node[i].host = x_strdup(str_val);
			} else {
				loger("(host) not found, under (data_set) idx[%d]", i);
				goto fail;
			}

			if (json_object_object_get_ex(tmp_obj, "port", &dn_obj)) {
				p_kset->data_node[i].port = json_object_get_int(dn_obj);
			} else {
				loger("(port) not found, under (data_set) idx[%d]", i);
				goto fail;
			}
		}
	} else {
		loger("(data_set) not found, under (key_set) [%d, %d]", p_kset->s_key, p_kset->e_key);
		goto fail;
	}

	return 0;

fail:
	return -1;
}

void read_timport_cfg(struct timport_cfg_file *p_cfg, char *name)
{
	int                     i = 0;
	int                     array_len = 0;
	const char              *str_val = NULL;
	timport_key_t           *p_tktree = &p_cfg->tktree;
	tsdb_set_t              *p_tsdb = &p_cfg->tsdb;
	struct json_object      *obj = NULL, *tmp_obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "max_req_size", &obj)) {
		p_cfg->max_req_size = json_object_get_int(obj);
	} else {
		loger("(max_req_size) not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "log_path", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->log_path = x_strdup(str_val);
	} else {
		loger("(log_path) not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "log_file", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->log_file = x_strdup(str_val);
	} else {
		loger("(log_file) not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "log_level", &obj)) {
		p_cfg->log_level = json_object_get_int(obj);
	} else {
		loger("(log_level) not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "delay_time", &obj)) {
		p_cfg->delay_time = json_object_get_int(obj);

		if (p_cfg->delay_time < MIN_DELAY_TIME) {
			loger("(delay_time) less than %d", MIN_DELAY_TIME);
		}
	} else {
		loger("(delay_time) not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "zk_disabled", &obj)) {
		p_cfg->zk_disabled = json_object_get_int(obj);

		if (p_cfg->zk_disabled) {
			loger("[WARN] (zk_disabled) == 1");
		}
	} else {
		loger("[INFO] (zk_disabled) not set");
	}

	if (json_object_object_get_ex(cfg, "redis", &obj)) {
		array_len = json_object_array_length(obj);

		if (array_len <= 0) {
			loger("(redis) array length err");
			goto fail;
		}

		NewArray0(array_len, p_cfg->redis);

		if (NULL == p_cfg->redis) {
			loger("(redis) NewArray0 err");
			goto fail;
		}

		p_cfg->redis_cnt = array_len;

		for (i = 0; i < array_len; ++i) {
			struct json_object *rds_obj = json_object_array_get_idx(obj, i);

			if (json_object_object_get_ex(rds_obj, "host", &tmp_obj)) {
				str_val = json_object_get_string(tmp_obj);
				p_cfg->redis[i].host = x_strdup(str_val);
			} else {
				loger("(redis) no (host)");
				goto fail;
			}

			if (json_object_object_get_ex(rds_obj, "port", &tmp_obj)) {
				p_cfg->redis[i].port = json_object_get_int(tmp_obj);
			} else {
				loger("(redis) no (port)");
				goto fail;
			}

			p_cfg->redis[i].idx = i;
		}
	} else {
		loger("(redis) not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "tsdb", &obj)) {
		array_len = json_object_array_length(obj);

		if (array_len > 32) {
			goto fail;
		}

		p_tsdb->kset_cnt = array_len;

		for (i = 0; i < array_len; ++i) {
			int ok = load_kset_from_jso(&p_tsdb->key_set[i], json_object_array_get_idx(obj, i), p_cfg);

			if (ok != 0) {
				loger("(tsdb) load_kset_from_jso err");
				goto fail;
			}
		}
	} else {
		if (p_cfg->zk_disabled) {
			loger("(zk_disabled) == 1, but no (tsdb)");
			goto fail;
		}
	}

	if (json_object_object_get_ex(cfg, "statistics", &obj)) {
		if (json_object_object_get_ex(obj, "host", &tmp_obj)) {
			str_val = json_object_get_string(tmp_obj);
			p_cfg->statistics.host = x_strdup(str_val);
		} else {
			loger("(statistics) has no (host)");
			goto fail;
		}

		if (json_object_object_get_ex(obj, "port", &tmp_obj)) {
			p_cfg->statistics.port = json_object_get_int(tmp_obj);
		} else {
			loger("(statistics) has no (host)");
			goto fail;
		}

		if (json_object_object_get_ex(obj, "keys", &tmp_obj)) {
			array_len = json_object_array_length(tmp_obj);

			if (array_len <= 0) {
				loger("(statistics) has 0 (keys)");
				goto fail;
			}

			NewArray0(array_len, p_cfg->statistics.keys);

			if (NULL == p_cfg->statistics.keys) {
				loger("(statistics) (keys) NewArray0 failed");
				goto fail;
			}

			p_cfg->statistics.keys_cnt = array_len;

			for (i = 0; i < array_len; ++i) {
				struct json_object *stat_obj = json_object_array_get_idx(tmp_obj, i);

				if (json_object_object_get_ex(stat_obj, "key", &obj)) {
					str_val = json_object_get_string(obj);
					p_cfg->statistics.keys[i].name = x_strdup(str_val);
				} else {
					loger("(statistics) (keys[%d]) has no (key)", i);
					goto fail;
				}

				if (json_object_object_get_ex(stat_obj, "mode", &obj)) {
					str_val = json_object_get_string(obj);

					if (strcasecmp(str_val, "INCR") == 0) {
						p_cfg->statistics.keys[i].mode = STAT_MODE_INCR;
					} else if (strcasecmp(str_val, "SET") == 0) {
						p_cfg->statistics.keys[i].mode = STAT_MODE_SET;
					} else {
						p_cfg->statistics.keys[i].mode = STAT_MODE_UNKNOWN;
					}
				}
			}
		} else {
			loger("(statistics) has no (keys)");
			goto fail;
		}
	}

	if (json_object_object_get_ex(cfg, "timport", &obj)) {
		array_len = json_object_array_length(obj);

		if (array_len <= 0) {
			loger("(timport) array length err");
			goto fail;
		}

		p_tktree->parent = NULL;

		NewArray0(array_len, p_tktree->child);

		if (NULL == p_tktree->child) {
			loger("(timport) NewArray0 err");
			goto fail;
		}

		p_tktree->child_cnt = array_len;

		for (i = 0; i < array_len; ++i) {
			p_tktree->child[i].parent = p_tktree;
			int ok = load_tkey_from_jso(&p_tktree->child[i], json_object_array_get_idx(obj, i), p_cfg);

			if (ok != 0) {
				loger("(timport) load_tkey_from_jso err");
				goto fail;
			}
		}
	} else {
		goto fail;
	}

	if (!p_cfg->zk_disabled) {
		if (json_object_object_get_ex(cfg, "zk_servers", &obj)) {
			str_val = json_object_get_string(obj);
			p_cfg->zk_servers = x_strdup(str_val);
		} else {
			loger("(zk_servers) not found");
			goto fail;
		}

		if (json_object_object_get_ex(cfg, "zk_rnode", &obj)) {
			str_val = json_object_get_string(obj);
			p_cfg->zk_rnode = x_strdup(str_val);
		} else {
			loger("(zk_rnode) not found");
			goto fail;
		}
	}

	if (json_object_object_get_ex(cfg, "backup_path", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->backup_path = x_strdup(str_val);
	} else {
		loger("(backup_path) not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "start_time_file", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->start_time_file = x_strdup(str_val);
	} else {
		loger("(start_time_file) not found");
		goto fail;
	}

	json_object_put(cfg);

	return;

fail:

	if (cfg) {
		json_object_put(cfg);
	}

	loger("invalid config file :%s", name);
	exit(EXIT_FAILURE);
}

