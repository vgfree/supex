/*
 * 版权声明：暂无
 * 文件名称：map_pmr.c
 * 创建者   ：
 * 创建日期：
 * 文件描述：
 * 历史记录：
 */
#include "map_pmr.h"
#include "pmr_cfg.h"
#include "map_grid.h"
#include "map_utils.h"
#include "map_errno.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>



static map_pmr_cfg g_pmr_cfg;

/*加在配置文件*/
int pmr_load_cfg(char *p_name)
{
	if (!p_name) {
		return -1;
	}

	pmr_load_cfg_file(p_name, &g_pmr_cfg);

	return 0;
}

/*加在单独line数据*/
int pmr_load_data_line()
{
	return map_line_load(g_pmr_cfg.map_line_file);
}

/*单独加在grid数据*/
int pmr_load_data_grid()
{
	return map_grid_load(g_pmr_cfg.map_grid_count_file, g_pmr_cfg.map_grid_line_file);
}

/*加在line和grid数据*/
int pmr_load_data_all()
{
	pmr_load_data_line();
	pmr_load_data_grid();

	return 0;
}

/*将BL扩展，用于BL筛选*/
static void extent_coor(double coor1, double coor2, double *p_min_coor, double *p_max_coor)
{
	double  min_coor = (coor1 > coor2) ? coor2 : coor1;
	double  max_coor = (coor1 > coor2) ? coor1 : coor2;

	*p_min_coor = min_coor - GRID_BORDER_LIMIT;
	*p_max_coor = max_coor + GRID_BORDER_LIMIT;
}

/*点在路上定位回调函数*/
static int pmr_locate_cb(void *arg, map_line_info *p_line_info)
{
	if (!arg || !p_line_info) {
		return ERR_PMR_PARAMETER;
	}

	match_line *p_lines = (match_line *)arg;

	/*方向角筛选*/
	if ((p_lines->pt_dir >= 0) && (direction_sub(p_lines->pt_dir, p_line_info->dir) > MAX_DIR_SUB)) {
		return ERR_PMR_DIR_FILTER;
	}

	/*BL筛选*/
	double  min_lon = 0;
	double  max_lon = 0;
	double  min_lat = 0;
	double  max_lat = 0;

	extent_coor(p_line_info->start_lon, p_line_info->end_lon, &min_lon, &max_lon);
	extent_coor(p_line_info->start_lat, p_line_info->end_lat, &min_lat, &max_lat);

	if ((p_lines->pt_lon < min_lon) || (p_lines->pt_lon > max_lon)
		|| (p_lines->pt_lat < min_lat) || (p_lines->pt_lat > max_lat)) {
		return ERR_PMR_BL_FILTER;
	}

	/*距离筛选*/
	int dist = dist_p2l(p_lines->pt_lon, p_lines->pt_lat, p_line_info->start_lon, p_line_info->start_lat,
			p_line_info->end_lon, p_line_info->end_lat);

	x_printf(I, "line:%d, dist:%d\n", p_line_info->line_id, dist);

	if (dist > MAX_DIST_P2L) {
		return ERR_PMR_DIST_FILTER;
	}

	if ((p_lines->match_size + 1) >= MAX_MATCH_LINE) {
		x_printf(E, "match to many lines!");
		return 0;
	}

	*(p_lines->lines + p_lines->match_size) = p_line_info;
	*(p_lines->dist + p_lines->match_size) = dist;
	p_lines->match_size++;

	return 0;
}

/*获取距离点最近的line*/
/*高速公路优先*/
map_line_info *get_nearest_line(match_line *p_lines)
{
	if (!p_lines || (p_lines->match_size == 0)) {
		return NULL;
	}

	int             i;
	int             min_dist = MAX_DIST_P2L;
	map_line_info   *p_res_line = NULL;

	for (i = 0; i < p_lines->match_size; i++) {
		if (min_dist >= p_lines->dist[i]) {
			min_dist = p_lines->dist[i];
			p_res_line = p_lines->lines[i];
		}
	}

	return p_res_line;
}

/*点在路上定位调用*/
int pmr_locate(map_line_info **pp_line, short direction, double longitude, double latitude)
{
	if (!pp_line) {
		return ERR_PMR_PARAMETER;
	}

	match_line m_lines;
	memset(&m_lines, 0, sizeof(m_lines));
	m_lines.match_size = 0;
	m_lines.pt_lon = longitude;
	m_lines.pt_lat = latitude;
	m_lines.pt_dir = direction;

	int ret = map_grid_query(longitude, latitude, pmr_locate_cb, (void *)&m_lines);

	if (ret < 0) {
		return ret;
	} else if (ret == 0) {
		return ERR_PMR_LOCATE_FAILED;
	}

	map_line_info *p_line = get_nearest_line(&m_lines);
	x_printf(I, "match lineid:%d", p_line->line_id);

	if (!p_line) {
		return ERR_PMR_LOCATE_FAILED;
	}

	*pp_line = p_line;

	return 0;
}
