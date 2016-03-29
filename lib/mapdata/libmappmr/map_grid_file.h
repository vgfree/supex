/*
 * 版权声明：暂无
 * 文件名称：map_grid_file.c
 * 创建者   ：耿玄玄
 * 创建日期：2015/10/20
 * 文件描述：load data from file
 * 历史记录：
 */

#pragma once

#include <stdint.h>
#include "pmr_cfg.h"
#include "map_grid_list.h"
#include "db_api.h"

/*网格数据管理结构体*/
typedef struct map_grid_manager
{
	int                     lon_begin;
	int                     lat_begin;
	unsigned int            lon_size;
	unsigned int            lat_size;
	map_grid_index_list     index_list;
	map_grid_list           grid_list;
	mysqlconn               grid_conn;
} map_grid_manager;

int32_t map_grid_load_file(char *grid_count_file, char *grid_line_file, map_grid_manager *p_manage, uint32_t step_len);

int32_t map_grid_gen_file(char *grid_count_file, char *grid_line_file, map_grid_load_cfg *p_cfg);

int32_t map_grid_manage_destory(map_grid_manager *p_manage);

