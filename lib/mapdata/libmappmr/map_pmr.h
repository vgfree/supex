/*
 * 版权声明：暂无
 * 文件名称：map_pmr.h
 * 创建者   ：耿玄玄
 * 创建日期：2015/10/19
 * 文件描述：用于点在路上pmr数据初始化及定位
 * 历史记录：
 */
#pragma once

#include "map_line.h"

#define MAX_MATCH_LINE  30
#define MAX_DIST_P2L    30	/*点到直线最大距离*/
#define MAX_DIR_SUB     20
typedef struct match_line
{
        int             match_size;
        map_line_info   *lines[MAX_MATCH_LINE];
        int             dist[MAX_MATCH_LINE];	/*当前点到该line距离*/
        double          pt_lon;			/*定位点经度*/
        double          pt_lat;			/*定位点纬度*/
        short           pt_dir;			/*定位点方向角*/
        short           pt_alt;			/*定位点海拔*/
        short       pl_spd;
} match_line;

/*
 * 名    称：pmr_load_cfg
 * 功    能：
 * 参    数：
 * 返回值：
 */
int pmr_load_cfg(char *p_name);

/*
 * 名    称：pmr_load_data_line
 * 功    能：
 * 参    数：
 * 返回值：
 */
int pmr_load_data_line();

/*
 * 名    称：pmr_load_data_grid
 * 功    能：
 * 参    数：
 * 返回值：
 */
int pmr_load_data_grid();

map_line_info *get_nearest_line(match_line *p_lines);

/*
 * 名    称：pmr_load_data_all
 * 功    能：
 * 参    数：
 * 返回值：
 */
int pmr_load_data_all();

/*
 * 名    称：pmr_locate
 * 功    能：
 * 参    数：
 * 返回值：
 */
int pmr_locate(map_line_info **, short direction, double longitude, double latitude);
void pmr_desory();

