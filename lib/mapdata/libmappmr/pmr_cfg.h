/*
 * 版权声明：暂无
 * 文件名称：pmr_cfg.h
 * 创建者   ：
 * 创建日期：
 * 文件描述：
 * 历史记录：
 */
#pragma once

/*加载map_line数据的配子信息*/
typedef struct map_line_load_cfg
{
	unsigned short  port;
	unsigned int    load_once;
	long            min_id;
	long            max_id;
	char            host[32];
	char            username[32];
	char            passwd[32];
	char            database[32];
	char            table[32];
} map_line_load_cfg;

/*加载map_grid数据的配置信息*/
typedef struct map_grid_load_cfg
{
	unsigned short  port;
	unsigned int    load_once;
	unsigned int    max_gridcount_id;
	unsigned int    max_gridline_id;
	double          min_lon;
	double          min_lat;
	double          max_lon;
	double          max_lat;
	char            host[32];
	char            username[32];
	char            passwd[32];
	char            database[32];
	char            count_table[32];
	char            line_table[32];
} map_grid_load_cfg;

/*点在路上pmr计算所需要的配置信息*/
typedef struct map_pmr_cfg
{
	unsigned short  max_dist;
	char            map_line_file[32];
	char            map_grid_count_file[32];
	char            map_grid_line_file[32];
} map_pmr_cfg;

/*
 * 名    称：pmr_load_cfg_file
 * 功    能：
 * 参    数：
 * 返回值：
 */
int pmr_load_cfg_file(char *p_name, map_pmr_cfg *p_pmr_cfg);

int pmr_load_cfg_mysql(char *p_name, map_line_load_cfg *p_line_cfg, map_grid_load_cfg *p_grid_cfg);

