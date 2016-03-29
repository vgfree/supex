/*
 * 版权声明：暂无
 * 文件名称：seg_cfg.c
 * 创建者   ：王张彦
 * 创建日期：2015/11/18
 * 文件描述：seg加载配置文件
 * 历史记录：
 */
#include <stdio.h>
#include <string.h>
#include "seg_cfg.h"
#include "json.h"
#include "libmini.h"

int seg_load_cfg_mysql(char *p_name, map_seg_cfg *p_seg_cfg)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *new_obj = NULL;
	struct json_object      *cfg = json_object_from_file(p_name);

	if (cfg == NULL) {
		x_printf(D, "cfg == NULL");
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "map_seg", &obj)) {
		if (json_object_object_get_ex(obj, "rrid_index_long", &new_obj)) {
			p_seg_cfg->index_long = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "rrid_buf_long", &new_obj)) {
			p_seg_cfg->rrid_buf_long = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "name_index_long", &new_obj)) {
			p_seg_cfg->name_index_long = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "name_buf_long", &new_obj)) {
			p_seg_cfg->name_buf_long = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "load_sgid_once", &new_obj)) {
			p_seg_cfg->load_once = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "min_index", &new_obj)) {
			p_seg_cfg->min_index = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "max_index", &new_obj)) {
			p_seg_cfg->max_index = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "port", &new_obj)) {
			p_seg_cfg->port = (unsigned short)json_object_get_int(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "host", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_seg_cfg->host, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "username", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_seg_cfg->username, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "password", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_seg_cfg->passwd, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "database", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_seg_cfg->database, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "table", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_seg_cfg->table, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "map_seg_file", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_seg_cfg->map_seg_file, str_val);
		} else { goto fail; }

		return 0;
	} else { goto fail; }

fail:
	x_printf(D, "invalid config file :%s\n", p_name);
	exit(1);
	return 0;
}

