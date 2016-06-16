/*版权声明：无
 * 文件名称：map_line.h
 * 创建者：王张彦
 * 创建日期：2015.10.19
 * 文件描述：
 *历史记录：无
 */

#pragma once
#include <stdlib.h>
#include <stdint.h>
#include "pmr_cfg.h"
#include "map_line_file.h"

#define GRID_BORDER_LIMIT 0.00025

// int map_line_load(map_line_load_cfg *p_cfg);
int map_line_load(char *file_name);

map_line_info *map_line_query(unsigned int line_id);

int map_line_destory();

