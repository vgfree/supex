#include "poi_cfg.h"
#include <stdlib.h>
#include "libmini.h"
#include "json.h"

int mappoi_load_cfg_mysql(mappoi_cfg_mysql *p_poi_cfg, char *p_name)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *new_obj = NULL;
	struct json_object      *cfg = json_object_from_file(p_name);

	if (cfg == NULL) {
		x_printf(D, "cfg == NULL");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "map_poi", &obj)) {
		if (json_object_object_get_ex(obj, "host", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_poi_cfg->host, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "port", &new_obj)) {
			p_poi_cfg->port = (unsigned short)json_object_get_int(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "username", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_poi_cfg->username, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "password", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_poi_cfg->passwd, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "database", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_poi_cfg->database, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "poi_table", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_poi_cfg->poi_table, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "sg_table", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_poi_cfg->sg_table, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "load_once", &new_obj)) {
			p_poi_cfg->load_once = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "max_sg_id", &new_obj)) {
			p_poi_cfg->max_sg_id = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "max_poi_id", &new_obj)) {
			p_poi_cfg->max_poi_id = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "file_name_poi", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_poi_cfg->file_name_poi, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "file_name_sg", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_poi_cfg->file_name_sg, str_val);
		} else { goto fail; }

		return 0;
	} else { goto fail; }

fail:
	x_printf(D, "invalid config file :%s\n", p_name);
	exit(1);
}

int mappoi_load_cfg_data(mappoi_cfg_file *p_poi_cfg, char *p_name)
{
	const char              *str_val = NULL;
	struct json_object      *new_obj = NULL;
	struct json_object      *cfg = json_object_from_file(p_name);

	if (cfg == NULL) {
		x_printf(D, "cfg == NULL");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "file_name_poi", &new_obj)) {
		str_val = json_object_get_string(new_obj);
		strcpy(p_poi_cfg->file_name_poi, str_val);
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "file_name_sg", &new_obj)) {
		str_val = json_object_get_string(new_obj);
		strcpy(p_poi_cfg->file_name_sg, str_val);
	} else { goto fail; }

	return 0;

fail:
	x_printf(D, "invalid config file :%s\n", p_name);
	exit(1);
}

