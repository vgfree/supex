/*
 * 版权声明：暂无
 * 文件名称：map_utils.h
 * 创建者   ：耿玄玄
 * 创建日期：2015/10/19
 * 文件描述：地图计算公共函数
 * 历史记录：
 */
#pragma once

/*
 * 名    称：get_coor_5thousand
 * 功    能：将经度或者纬度转换为千分之五网格的坐标
 * 参    数：经度或者纬度
 * 返回值：转换后的数据
 */
int get_coor_5thousand(double coor1);

int convert_grid_id(char *grid_id, unsigned int *lon_num, unsigned int *lat_num);

/*计算地球点到点的距离*/
unsigned int dist_p2p(double from_lon, double from_lat, double to_lon, double to_lat);

/* 计算地球点到直线的距离 */
unsigned int dist_p2l(double pt_lon, double pt_lat, double line_st_lon, double line_st_lat, double line_ed_lon, double line_ed_lat);

unsigned short direction_sub(unsigned short dir1, unsigned short dir2);

/* 两直线的夹角，返回角度值，非弧度值 */
double IncludedAngle(long line1_x, long line1_y,long line2_x, long line2_y);

/*判断gps点是否在之前范围内*/
int is_in_line_range(double pt_lon, double pt_lat, double sl, double sb, double el, double eb);

