#include "topo_cfg.h"
#include <stdlib.h>
#include "json.h"
#include "libmini.h"

int topo_cfg_init_mysql(char *p_name, topo_cfg_mysql_t *p_cfg)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *new_obj = NULL;
	struct json_object      *cfg = json_object_from_file(p_name);

	if (cfg == NULL) {
		x_printf(D, "cfg == NULL");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "map_topo", &obj)) {
		if (json_object_object_get_ex(obj, "load_once", &new_obj)) {
			p_cfg->load_once = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "max_id", &new_obj)) {
			p_cfg->max_id = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "topo_file_name", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_cfg->map_topo_file, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "port", &new_obj)) {
			p_cfg->port = (unsigned short)json_object_get_int(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "host", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_cfg->host, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "username", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_cfg->username, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "password", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_cfg->passwd, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "database", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_cfg->database, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "table", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_cfg->table, str_val);
		} else { goto fail; }

		return 0;
	} else { goto fail; }

fail:
	x_printf(D, "invalid config file :%s\n", p_name);
	exit(1);
	return 0;
}

int topo_cfg_init_file(char *p_name, topo_cfg_file_t *p_cfg)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *new_obj = NULL;

	struct json_object *cfg = json_object_from_file(p_name);

	if (cfg == NULL) {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "map_topo", &obj)) {
		if (json_object_object_get_ex(obj, "topo_file_name", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			sprintf(p_cfg->topo_file, "%s", str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "rrid_index_len", &new_obj)) {
			p_cfg->rrid_index_len = (long)json_object_get_int64(new_obj);
		} else { goto fail; }
	} else { goto fail; }

	return 0;

fail:
	x_printf(E, "invalid config file :%s\n", p_name);
	exit(1);
}

