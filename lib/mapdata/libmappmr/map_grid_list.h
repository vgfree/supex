#pragma once

#include <stdint.h>
#include "libmini.h"

/*存储网格具体信息结构体*/
typedef struct map_grid_info
{
	uint16_t        line_size;
	uint16_t        max_size;
	int32_t         grid_lon;
	int32_t         grid_lat;
	uint32_t        *p_lines;
} map_grid_info;

/*存储网格数据的静态二维数组*/
typedef struct map_grid_list
{
	uint16_t        list_len;
	uint32_t        step_len;
	uint32_t        max_size;
	map_grid_info   **p_list;
} map_grid_list;

/*存储网格索引的静态数组，可根据经纬度转换为数组索引查询*/
typedef struct map_grid_index_list
{
	uint16_t        list_len;
	uint32_t        step_len;
	uint32_t        max_size;
	map_grid_info   ***p_list;
} map_grid_index_list;

int32_t map_grid_list_init(map_grid_list *p_grid_list, uint32_t max_size, uint32_t step_len);

int32_t map_grid_index_list_init(map_grid_index_list *p_index_list, uint32_t max_size, uint32_t step_len);

/*添加grid信息到静态内存中，并返回静态的line指针，不要修改*/
map_grid_info *map_grid_list_add(map_grid_list *p_grid_list, map_grid_info *p_info, uint32_t index);

/*添加静态grid指针到静态索引中*/
int32_t map_grid_index_list_add(map_grid_index_list *p_index_list, map_grid_info *p_info, uint32_t index);

/*根据经纬度索引查找mapgrid*/
map_grid_info *find_map_grid_index(map_grid_index_list *p_grid_list, uint32_t index);

