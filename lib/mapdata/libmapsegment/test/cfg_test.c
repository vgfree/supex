/*
 * 版权声明：暂无
 * 文件名称：seg_test.c
 * 创建者  ：wangzhangyan
 * 创建日期：2015/11/18
 * 文件描述：测试配置文件
 * 历史记录：无
 */

#include <stdio.h>
#include "seg_cfg.h"
#include "libmini.h"

int main()
{
	// x_printf(D, "seg_cfg-test");
	map_seg_cfg seg_cfg;

	seg_load_cfg_mysql("SEG_conf.json", &seg_cfg);

	x_printf(D, "=================================\n");
	x_printf(D, "index_long========%ld\n", seg_cfg.index_long);
	x_printf(D, "rrid_buf_long=====%ld\n", seg_cfg.rrid_buf_long);
	x_printf(D, "name_index_long===%d\n", seg_cfg.name_index_long);
	x_printf(D, "name_buf_long=====%d\n", seg_cfg.name_buf_long);
	x_printf(D, "port==============%d\n", seg_cfg.port);
	x_printf(D, "host==============%s\n", seg_cfg.host);
	x_printf(D, "username==========%s\n", seg_cfg.username);
	x_printf(D, "passwd============%s\n", seg_cfg.passwd);
	x_printf(D, "database==========%s\n", seg_cfg.database);
	x_printf(D, "table=============%s\n", seg_cfg.table);
	x_printf(D, "map_seg_file======%s\n", seg_cfg.map_seg_file);
	x_printf(D, "load_once=========%ld\n", seg_cfg.load_once);
	x_printf(D, "max_index=========%ld\n", seg_cfg.max_index);
	x_printf(D, "min_index=========%ld\n", seg_cfg.min_index);
	x_printf(D, "=================================\n");

	return 0;
}

