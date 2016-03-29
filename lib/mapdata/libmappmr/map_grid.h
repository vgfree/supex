#pragma once

#include "map_line.h"

/*
 * 名    称：PMR_GRID_CB
 * 功    能：判断点在路上判断的回调函数
 * 参    数：  arg回调传入的指针
 *                  p_line_info--迭代的map_line_info指针
 * 返回值：0--满足条件的line
 *                其他--错误代码
 */
typedef int (*PMR_GRID_CB)(void *arg, map_line_info *p_line_info);

/*
 * 名    称：map_grid_load
 * 功    能：加载grid数据
 * 参    数：p_cfg--配置文件中的配置信息
 * 返回值：0：加载数据成功
 *                其他--错误代码
 */
int map_grid_load(char *grid_count_file, char *grid_line_file);

/*
 * 名    称：map_grid_query
 * 功    能：遍历传入坐标周边网格的所有line
 * 参    数：lon：纬度
 *                lat：经度
 *                func_pmr_cb: 满足条件的line回调函数
 *                arg:传递给回调函数的指针
 * 返回值：符合回调函数的line个数
 */
int map_grid_query(double lon, double lat, PMR_GRID_CB func_pmr_cb, void *arg);

/*
 * 名    称：map_grid_destory
 * 功    能：销毁grid数据
 * 参    数：无
 * 返回值：无
 */
void map_grid_destory();

