/*
 * 版权声明：暂无
 * 文件名称：pmr_test.c
 * 创建者  ：滕兴惠
 * 创建日期：2015/10/20
 * 文件描述：测试”点在路上“的配置文件
 * 历史记录：无
 */

#include <stdio.h>
#include "pmr_cfg.h"
#include "libmini.h"

int main()
{
	map_line_load_cfg       line_cfg;
	map_grid_load_cfg       grid_cfg;
	map_pmr_cfg             pmr_cfg;

	pmr_load_cfg_mysql("PMR_conf.json", &line_cfg, &grid_cfg);
	pmr_load_cfg_file("PMR_conf.json", &pmr_cfg);

	x_printf(D, "=================================\n");

	x_printf(D, "min_id============%ld\n", line_cfg.min_id);
	x_printf(D, "max_id============%ld\n", line_cfg.max_id);
	x_printf(D, "port==============%d\n", line_cfg.port);
	x_printf(D, "host==============%s\n", line_cfg.host);
	x_printf(D, "username==========%s\n", line_cfg.username);
	x_printf(D, "passwd============%s\n", line_cfg.passwd);
	x_printf(D, "database==========%s\n", line_cfg.database);
	x_printf(D, "table=============%s\n", line_cfg.table);

	x_printf(D, "=================================\n");

	x_printf(D, "port==============%d\n", grid_cfg.port);
	x_printf(D, "load_once=========%d\n", grid_cfg.load_once);
	x_printf(D, "max_gridcount_id==%d\n", grid_cfg.max_gridcount_id);
	x_printf(D, "max_gridline_id===%d\n", grid_cfg.max_gridline_id);
	x_printf(D, "min_lon===========%f\n", grid_cfg.min_lon);
	x_printf(D, "min_lat===========%f\n", grid_cfg.min_lat);
	x_printf(D, "max_lon===========%f\n", grid_cfg.max_lon);
	x_printf(D, "max_lat===========%f\n", grid_cfg.max_lat);
	x_printf(D, "host==============%s\n", grid_cfg.host);
	x_printf(D, "username==========%s\n", grid_cfg.username);
	x_printf(D, "passwd============%s\n", grid_cfg.passwd);
	x_printf(D, "database==========%s\n", grid_cfg.database);
	x_printf(D, "count_table=======%s\n", grid_cfg.count_table);
	x_printf(D, "line_table========%s\n", grid_cfg.line_table);

	x_printf(D, "=================================\n");

	x_printf(D, "max_dist=%d\n", pmr_cfg.max_dist);

	return 0;
}

