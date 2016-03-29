#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "tsdb_cfg.h"
#include "json.h"
#include "utils.h"

void read_tsdb_cfg(struct tsdb_cfg_file *p_cfg, char *name)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL, *tmp_obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	/* engine_type */
	if (json_object_object_get_ex(cfg, "engine_type", &obj)) {
		str_val = json_object_get_string(obj);

		if (strcasecmp(str_val, "LDB") == 0) {
			p_cfg->engine_type = TSDB_ENGINE_LDB;
		} else if (strcasecmp(str_val, "KV") == 0) {
			p_cfg->engine_type = TSDB_ENGINE_KV;
		} else if (strcasecmp(str_val, "MIX") == 0) {
			p_cfg->engine_type = TSDB_ENGINE_MIX;
		} else {
			x_printf(F, "config: engine_type error, (LDB|KV|MIX)");
			goto fail;
		}
	} else {
		x_printf(W, "config: engine_type set to LDB!");
		p_cfg->engine_type = TSDB_ENGINE_LDB;
	}

	/* node type. */
	if (json_object_object_get_ex(cfg, "node_type", &obj)) {
		str_val = json_object_get_string(obj);

		if (strcmp(str_val, "SINGLE") == 0) {
			p_cfg->node_type = SINGLE;
		} else if (strcmp(str_val, "CLUSTER") == 0) {
			p_cfg->node_type = CLUSTER;
		} else {
			x_printf(F, "config: node_type error");
			goto fail;
		}
	} else {
		x_printf(F, "config: node_type not found");
		goto fail;
	}

	/* mode. */
	if (json_object_object_get_ex(cfg, "mode", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->mode = x_strdup(str_val);
	} else {
		x_printf(F, "config: mode not found");
		goto fail;
	}

	if (strcmp(p_cfg->mode, "RO") == 0) {
		p_cfg->ldb_readonly_switch = 1;
	} else if (strcmp(p_cfg->mode, "RW") == 0) {
		p_cfg->ldb_readonly_switch = 0;
	} else {
		x_printf(F, "config: mode error");
		goto fail;
	}

	/* data set id. */
	if (json_object_object_get_ex(cfg, "ds_id", &obj)) {
		p_cfg->ds_id = json_object_get_int(obj);
	} else {
		x_printf(F, "config: ds_id not found");
		goto fail;
	}

	/* key set 2 */
	if (json_object_object_get_ex(cfg, "key_set", &obj)) {
		if (json_object_array_length(obj) != 2) {
			x_printf(F, "config: key_set2 error");
			goto fail;
		}

		tmp_obj = json_object_array_get_idx(obj, 0);
		p_cfg->key_start = json_object_get_int(tmp_obj);
		tmp_obj = json_object_array_get_idx(obj, 1);
		p_cfg->key_end = json_object_get_int(tmp_obj);
	} else {
		x_printf(F, "config: key_set not found");
		goto fail;
	}

	/* time range. */
	if (json_object_object_get_ex(cfg, "time_range", &obj)) {
		if (json_object_array_length(obj) != 2) {
			x_printf(F, "config: time_range error");
			goto fail;
		}

		tmp_obj = json_object_array_get_idx(obj, 0);
		p_cfg->start_time = json_object_get_int64(tmp_obj);

		tmp_obj = json_object_array_get_idx(obj, 1);
		p_cfg->end_time = json_object_get_int64(tmp_obj);
	} else {
		x_printf(F, "config: time_range not found");
		goto fail;
	}

	if (p_cfg->ldb_readonly_switch == 1) {
		if (p_cfg->start_time > p_cfg->end_time) {
			x_printf(F, "config: start_time > end_time");
			goto fail;
		}
	} else if (p_cfg->ldb_readonly_switch == 0) {
		if (p_cfg->end_time != -1) {
			x_printf(F, "config: end_time error");
			goto fail;
		}
	}

	/* zookeeper server. */
	if (json_object_object_get_ex(cfg, "zk_server", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->zk_server = x_strdup(str_val);
	} else {
		x_printf(F, "config: zk_server not found");
		goto fail;
	}

	/* IP address. */
	if (json_object_object_get_ex(cfg, "ip", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->ip = x_strdup(str_val);
	} else {
		x_printf(F, "config: ip not found");
		goto fail;
	}

	/* write and read port. */
	if (json_object_object_get_ex(cfg, "w_port", &obj)) {
		p_cfg->w_port = json_object_get_int(obj);
	} else {
		x_printf(F, "config: w_port not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "r_port", &obj)) {
		p_cfg->r_port = json_object_get_int(obj);
	} else {
		x_printf(F, "config: r_port not found");
		goto fail;
	}

	/* workspace directory. */
	if (json_object_object_get_ex(cfg, "work_path", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->work_path = x_strdup(str_val);
	} else {
		x_printf(F, "config: work_path not found");
		goto fail;
	}

	/* ldb configure. */
	if (json_object_object_get_ex(cfg, "ldb_write_buffer_size", &obj)) {
		p_cfg->ldb_write_buffer_size = json_object_get_int64(obj);
	} else {
		x_printf(F, "config: ldb_write_buffer_size not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "ldb_block_size", &obj)) {
		p_cfg->ldb_block_size = json_object_get_int64(obj);
	} else {
		x_printf(F, "config: ldb_block_size not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "ldb_cache_lru_size", &obj)) {
		p_cfg->ldb_cache_lru_size = json_object_get_int64(obj);
	} else {
		x_printf(F, "config: ldb_cache_lru_size not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "ldb_bloom_key_size", &obj)) {
		p_cfg->ldb_bloom_key_size = json_object_get_int(obj);
	} else {
		x_printf(F, "config: ldb_bloom_key_size not found");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "ldb_compaction_speed", &obj)) {
		p_cfg->ldb_compaction_speed = json_object_get_int(obj);
	} else {
		x_printf(F, "config: ldb_compaction_speed not found");
		goto fail;
	}

	/* slave. */
	if (json_object_object_get_ex(cfg, "has_slave", &obj)) {
		p_cfg->has_slave = json_object_get_int(obj);
	} else {
		x_printf(F, "config: has_slave not found");
		goto fail;
	}

	if (p_cfg->has_slave != 0) {
		if (json_object_object_get_ex(cfg, "role", &obj)) {
			str_val = json_object_get_string(obj);

			if (strcmp(str_val, "MASTER") == 0) {
				p_cfg->role = MASTER;
			} else if (strcmp(str_val, "SLAVE") == 0) {
				p_cfg->role = SLAVE;
			} else {
				x_printf(F, "config: role config error");
				goto fail;
			}
		} else {
			x_printf(F, "config: role not found");
			goto fail;
		}

		if (json_object_object_get_ex(cfg, "slave_ip", &obj)) {
			str_val = json_object_get_string(obj);
			p_cfg->slave_ip = x_strdup(str_val);
		} else {
			x_printf(F, "config: slave_ip not found");
			goto fail;
		}

		if (json_object_object_get_ex(cfg, "slave_wport", &obj)) {
			p_cfg->slave_wport = json_object_get_int(obj);
		} else {
			x_printf(F, "config: slave_wport not found");
			goto fail;
		}
	}

	json_object_put(cfg);

	return;

fail:

	if (cfg) {
		json_object_put(cfg);
	}

	x_printf(F, "invalid config file :%s", name);
	exit(EXIT_FAILURE);
}

