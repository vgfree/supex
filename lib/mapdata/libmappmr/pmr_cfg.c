/*
 * 版权声明：暂无
 * 文件名称：pmr_cfg.c
 * 创建者  ：滕兴惠
 * 创建日期：2015/10/20
 * 文件描述：加载点在路上的配置信息
 * 历史记录：无
 */
#include <stdio.h>
#include <string.h>
#include "pmr_cfg.h"
#include "json.h"
#include "libmini.h"

/*
 * 名    称：pmr_load_cfg_file
 * 功    能：从json文件加载点在路上配置信息
 * 参    数：p_name,json文件名
 *           p_line_cfg,map_line数据配置
 *           p_grid_cfg,map_grid数据配置
 *           p_pmr_cfg,点在路上计算所需要的配置信息
 * 返回值：无
 */
int pmr_load_cfg_file(char *p_name, map_pmr_cfg *p_pmr_cfg)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *new_obj = NULL;

	struct json_object *cfg = json_object_from_file(p_name);

	if (cfg == NULL) {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "mappmr", &obj)) {
		if (json_object_object_get_ex(obj, "map_line_file", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_pmr_cfg->map_line_file, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "map_grid_count_file", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_pmr_cfg->map_grid_count_file, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "map_grid_line_file", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_pmr_cfg->map_grid_line_file, str_val);
		} else { goto fail; }
	} else { goto fail; }

	return 0;

fail:
	x_printf(E, "invalid config file :%s\n", p_name);
	exit(1);
}

int pmr_load_cfg_mysql(char *p_name, map_line_load_cfg *p_line_cfg, map_grid_load_cfg *p_grid_cfg)
{
	const char              *str_val = NULL;
	struct json_object      *obj = NULL;
	struct json_object      *new_obj = NULL;

	struct json_object *cfg = json_object_from_file(p_name);

	if (cfg == NULL) {
		goto fail;
	}

	if (json_object_object_get_ex(cfg, "map_line", &obj)) {
		if (json_object_object_get_ex(obj, "min_id", &new_obj)) {
			p_line_cfg->min_id = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "max_id", &new_obj)) {
			p_line_cfg->max_id = (long)json_object_get_int64(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "port", &new_obj)) {
			p_line_cfg->port = (unsigned short)json_object_get_int(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "host", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_line_cfg->host, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "username", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_line_cfg->username, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "password", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_line_cfg->passwd, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "database", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_line_cfg->database, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "table", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_line_cfg->table, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "load_line_once", &new_obj)) {
			p_line_cfg->load_once = (unsigned int)json_object_get_int(new_obj);
		} else { goto fail; }
	} else { goto fail; }

	if (json_object_object_get_ex(cfg, "map_grid", &obj)) {
		if (json_object_object_get_ex(obj, "port", &new_obj)) {
			p_grid_cfg->port = (unsigned short)json_object_get_int(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "load_grid_once", &new_obj)) {
			p_grid_cfg->load_once = (unsigned int)json_object_get_int(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "max_gridcount_id", &new_obj)) {
			p_grid_cfg->max_gridcount_id = (unsigned int)json_object_get_int(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "max_gridline_id", &new_obj)) {
			p_grid_cfg->max_gridline_id = (unsigned int)json_object_get_int(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "min_lon", &new_obj)) {
			p_grid_cfg->min_lon = json_object_get_double(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "min_lat", &new_obj)) {
			p_grid_cfg->min_lat = json_object_get_double(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "max_lon", &new_obj)) {
			p_grid_cfg->max_lon = json_object_get_double(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "max_lat", &new_obj)) {
			p_grid_cfg->max_lat = json_object_get_double(new_obj);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "host", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_grid_cfg->host, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "username", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_grid_cfg->username, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "password", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_grid_cfg->passwd, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "database", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_grid_cfg->database, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "count_table", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_grid_cfg->count_table, str_val);
		} else { goto fail; }

		if (json_object_object_get_ex(obj, "line_table", &new_obj)) {
			str_val = json_object_get_string(new_obj);
			strcpy(p_grid_cfg->line_table, str_val);
		} else { goto fail; }
	} else { goto fail; }

	return 0;

fail:
	x_printf(E, "invalid config file :%s\n", p_name);
	exit(1);
}

