#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "load_cfg.h"
#include "json.h"

#ifdef OPEN_ZOOKEEPER
int     ZK_DISABLED = 0;
char    *ZK_SERVERS = NULL;
char    *ZK_RNODE = NULL;
#endif

void load_cfg_file(struct smart_cfg_file *p_cfg, char *name)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *cfg = json_object_from_file(name);

	if (cfg == NULL) {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "service_port", &obj)) {
		p_cfg->srv_port = json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "hander_counts", &obj)) {
		p_cfg->hander_counts = (short)json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "smart_worker_counts", &obj)) {
		p_cfg->worker_counts = (short)json_object_get_int(obj);
	} else { goto fail; }

#if defined(OPEN_SCCO) || defined(OPEN_EVCORO)
	if (json_object_object_get_ex(cfg, "tasker_counts", &obj)) {
		p_cfg->tasker_counts = (short)json_object_get_int(obj);
	} else { goto fail; }
#endif

	if (json_object_object_get_ex(cfg, "smart_protocol", &obj)) {
		str_val = json_object_get_string(obj);

		if (strncmp(str_val, "http", 4) == 0) {
			p_cfg->ptype = USE_HTTP_PROTO;
		} else {
			p_cfg->ptype = USE_REDIS_PROTO;
		}
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "max_req_size", &obj)) {
		p_cfg->max_req_size = json_object_get_int(obj);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_path", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->log_path = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_file", &obj)) {
		str_val = json_object_get_string(obj);
		p_cfg->log_file = x_strdup(str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "log_level", &obj)) {
		p_cfg->log_level = json_object_get_int(obj);
	} else { goto fail; }

	if (p_cfg->ptype == USE_HTTP_PROTO) {
		if (json_object_object_get_ex(cfg, "api_apply", &obj)) {
			p_cfg->api_counts++;
			str_val = json_object_get_string(obj);
			p_cfg->api_apply = x_strdup(str_val);
		}

		if (json_object_object_get_ex(cfg, "api_fetch", &obj)) {
			p_cfg->api_counts++;
			str_val = json_object_get_string(obj);
			p_cfg->api_fetch = x_strdup(str_val);
		}

		if (json_object_object_get_ex(cfg, "api_merge", &obj)) {
			p_cfg->api_counts++;
			str_val = json_object_get_string(obj);
			p_cfg->api_merge = x_strdup(str_val);
		}

		if (json_object_object_get_ex(cfg, "api_custom", &obj)) {
			int add = json_object_array_length(obj);
			p_cfg->api_counts += add;
			assert(p_cfg->api_counts <= MAX_API_COUNTS);

			int                     i = 0;
			struct json_object      *itr_obj = NULL;
			memset(p_cfg->api_names, 0, MAX_API_COUNTS * (MAX_API_NAME_LEN + 1));

			for (i = 0; i < add; i++) {
				itr_obj = json_object_array_get_idx(obj, i);
				str_val = json_object_get_string(itr_obj);
				assert(strlen(str_val) <= MAX_API_NAME_LEN);
				strncpy(&p_cfg->api_names[i][0], str_val, MIN(strlen(str_val), MAX_API_NAME_LEN));
			}
		}
	}

#ifdef OPEN_ZOOKEEPER
	if (json_object_object_get_ex(cfg, "zk_disabled", &obj)) {
		ZK_DISABLED = json_object_get_int(obj);
	} else {
		ZK_DISABLED = 0;
	}

	if (!ZK_DISABLED) {
		if (json_object_object_get_ex(cfg, "zk_servers", &obj)) {
			str_val = json_object_get_string(obj);
			ZK_SERVERS = x_strdup(str_val);
		} else {
			x_printf(E, "(zk_servers) not found");
			goto fail;
		}

		if (json_object_object_get_ex(cfg, "zk_rnode", &obj)) {
			str_val = json_object_get_string(obj);
			ZK_RNODE = x_strdup(str_val);
		} else {
			x_printf(E, "(zk_rnode) not found");
			goto fail;
		}
	}
#endif	/* ifdef OPEN_ZOOKEEPER */

	return;

fail:
	x_printf(D, "invalid config file :%s", name);
	exit(EXIT_FAILURE);
}

