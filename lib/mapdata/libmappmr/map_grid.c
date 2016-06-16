/*
 * 版权声明：暂无
 * 文件名称：map_grid.c
 * 创建者   ：耿玄玄
 * 创建日期：2015/10/20
 * 文件描述：
 * 历史记录：
 */
#include <stdio.h>
#include <stdlib.h>
#include "map_grid.h"
#include "map_utils.h"
#include "map_errno.h"
#include "map_grid_file.h"
#include <assert.h>

#define GRID_MAX_LINE 10
static map_grid_manager g_grid_manager = {};

/* 加载grid数据 */
int map_grid_load(char *grid_count_file, char *grid_line_file)
{
	map_grid_load_file(grid_count_file, grid_line_file, &g_grid_manager, 10000);

	return 0;
}

/*将grid的经纬度插入到数组中*/
static int insert_grid(int lon_num, int lat_num, int *lon_array, int *lat_array, unsigned short size, unsigned short *p_count)
{
	if (!lon_array || !lat_array || !p_count || (size <= 0) || (*p_count >= size)) {
		return ERR_PMR_PARAMETER;
	}

	if ((lon_num < g_grid_manager.lon_begin) || (lon_num >= (g_grid_manager.lon_begin + g_grid_manager.lon_size * 5)) ||
		(lat_num < g_grid_manager.lat_begin) || (lat_num >= (g_grid_manager.lat_begin + g_grid_manager.lat_size * 5))) {
		return ERR_PMR_OUT_RANGE;
	}

	*(lon_array + *p_count) = lon_num;
	*(lat_array + *p_count) = lat_num;
	(*p_count)++;

	x_printf(D, "insert grid:%d&%d\n", lon_num, lat_num);

	return 0;
}

/*根据gps数据获取覆盖范围的网格*/
static int get_grids(double lon, double lat, int *lon_array, int *lat_array, unsigned short size, unsigned short *p_count)
{
	if (!lon_array || !lat_array || !p_count || (size <= 0) || (*p_count >= size)) {
		return ERR_PMR_PARAMETER;
	}

	int     lon_num = get_coor_5thousand(lon);
	int     lat_num = get_coor_5thousand(lat);
	*p_count = 0;

	/*超过当前网格范围*/
	if ((lon_num < g_grid_manager.lon_begin) ||
		((lon_num - g_grid_manager.lon_begin) / 5 >= g_grid_manager.lon_size) ||
		(lat_num < g_grid_manager.lat_begin) ||
		((lat_num - g_grid_manager.lat_begin) / 5 >= g_grid_manager.lat_size)) {
		return ERR_PMR_OUT_RANGE;
	}

	double  sub_lon_min = lon - lon_num / 1000.0;		/*当前gps经度与网格左方边缘之差*/
	double  sub_lat_min = lat - lat_num / 1000.0;		/*当前gps纬度与网格下方边缘之差*/
	double  sub_lon_max = (lon_num + 5) / 1000.0 - lon;	/*当前gps经度与网格右方边缘之差*/
	double  sub_lat_max = (lat_num + 5) / 1000.0 - lat;	/*当前gps纬度与网格上方边缘之差*/

	insert_grid(lon_num, lat_num, lon_array, lat_array, size, p_count);

	if (sub_lon_min < GRID_BORDER_LIMIT) {
		/*插入左中网格*/
		insert_grid(lon_num - 5, lat_num, lon_array, lat_array, size, p_count);
	} else if (sub_lon_max < GRID_BORDER_LIMIT) {
		/*插入右中网格*/
		insert_grid(lon_num + 5, lat_num, lon_array, lat_array, size, p_count);
	}

	if (sub_lat_min < GRID_BORDER_LIMIT) {
		/*插入下中网格*/
		insert_grid(lon_num, lat_num - 5, lon_array, lat_array, size, p_count);
	} else if (sub_lat_max < GRID_BORDER_LIMIT) {
		/*插入上中网格*/
		insert_grid(lon_num, lat_num + 5, lon_array, lat_array, size, p_count);
	}

	if ((sub_lon_min < GRID_BORDER_LIMIT) && (sub_lat_min < GRID_BORDER_LIMIT)) {
		/*插入左下网格*/
		insert_grid(lon_num - 5, lat_num - 5, lon_array, lat_array, size, p_count);
	} else if ((sub_lon_min < GRID_BORDER_LIMIT) && (sub_lat_max < GRID_BORDER_LIMIT)) {
		/*插入左上网格*/
		insert_grid(lon_num - 5, lat_num + 5, lon_array, lat_array, size, p_count);
	} else if ((sub_lon_max < GRID_BORDER_LIMIT) && (sub_lat_min < GRID_BORDER_LIMIT)) {
		/*插入右下网格*/
		insert_grid(lon_num + 5, lat_num - 5, lon_array, lat_array, size, p_count);
	} else if ((sub_lon_max < GRID_BORDER_LIMIT) && (sub_lat_max < GRID_BORDER_LIMIT)) {
		/*插入右上网格*/
		insert_grid(lon_num + 5, lat_num + 5, lon_array, lat_array, size, p_count);
	}

	return 0;
}

/* 遍历传入坐标周边网格的所有line*/
int map_grid_query(double lon, double lat, PMR_GRID_CB func_pmr_cb, void *arg)
{
	unsigned int    line_count = 0;
	unsigned short  grid_count = 0;

	int     grid_lon[GRID_MAX_LINE];
	int     grid_lat[GRID_MAX_LINE];

	int ret = get_grids(lon, lat, grid_lon, grid_lat, GRID_MAX_LINE, &grid_count);

	if (0 != ret) {
		return ret;
	}

	int i;

	for (i = 0; i < grid_count; i++) {
		int32_t         grid_index = (grid_lat[i] - g_grid_manager.lat_begin) / 5 * g_grid_manager.lon_size + (grid_lon[i] - g_grid_manager.lon_begin) / 5;
		map_grid_info   *p_grid_info = find_map_grid_index(&g_grid_manager.index_list, grid_index);
		x_printf(D, "map_grid_query grid:%d&%d, index:%d,ptr:%p\n", grid_lon[i], grid_lat[i], grid_index, p_grid_info);

		if (!p_grid_info) {
			continue;
		}

		int line_idx;

		for (line_idx = 0; line_idx < p_grid_info->line_size; line_idx++) {
			unsigned int    *p_line = p_grid_info->p_lines + line_idx;
			map_line_info   *p_line_info = map_line_query(*p_line);

			if (0 == func_pmr_cb(arg, p_line_info)) {
				line_count++;
			}
		}
	}

	return line_count;
}

/*名    称：map_grid_destory*/
void map_grid_destory()
{
	map_grid_manage_destory(&g_grid_manager);
}

