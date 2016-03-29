/*
 * 版权声明：暂无
 * 文件名称：map_utils.c
 * 创建者   ：耿玄玄
 * 创建日期：2015/10/19
 * 文件描述：地图计算公共函数
 * 历史记录：
 */
#include "map_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define EARTH_RADIUS 6378.137
#define PI 3.1415926535898

/*将经度或者纬度转换为千分之五网格的坐标*/
int get_coor_5thousand(double coor1)
{
	double  tmp100 = coor1 * 100;
	int     coor_int = (int)tmp100;
	double  coor_dec = tmp100 - coor_int;

	if ((coor_dec >= 0) && (coor_dec < 0.5)) {
		return coor_int * 10;
	} else {
		return coor_int * 10 + 5;
	}

	return 0;
}

int convert_grid_id(char *p_grid_id, unsigned int *p_lon_num, unsigned int *p_lat_num)
{
	if (!p_grid_id || !p_lon_num || !p_lat_num) {
		return -1;
	}

	char    *ptr = NULL;
	char    *p;
	ptr = strtok_r(p_grid_id, "&", &p);
	*p_lon_num = atol(ptr);

	if (!ptr) {
		return -1;
	}

	ptr = NULL;
	ptr = strtok_r(NULL, "&", &p);
	*p_lat_num = atol(ptr);

	if (!ptr) {
		return -1;
	}

	return 0;
}

/*计算地球点到点的距离*/
unsigned int dist_p2p(double from_lon, double from_lat, double to_lon, double to_lat)
{
	if ((fabs(from_lon - to_lon) < 0.00001) && (fabs(from_lat - to_lat) < 0.00001)) {
		return 0;
	}

	double distance = 0.0;
	distance = EARTH_RADIUS * acos(sin(from_lat / 57.2958) * sin(to_lat / 57.2958) + cos(from_lat / 57.2958) * cos(to_lat / 57.2958) * cos((from_lon - to_lon) / 57.2958)) * 1000;

	return floor(distance);
}

/* 计算地球点到直线的距离 */
unsigned int dist_p2l(double pt_lon, double pt_lat, double line_st_lon, double line_st_lat, double line_ed_lon, double line_ed_lat)
{
	unsigned int    a = dist_p2p(pt_lon, pt_lat, line_st_lon, line_st_lat);
	unsigned int    b = dist_p2p(pt_lon, pt_lat, line_ed_lon, line_ed_lat);
	unsigned int    c = dist_p2p(line_st_lon, line_st_lat, line_ed_lon, line_ed_lat);
	unsigned int    p = (a + b + c) / 2;
	unsigned int    dist = 2 * sqrt(abs(p * (p - a) * (p - b) * (p - c))) / c;

	return dist;
	//return (abs(a * a - b * b) > c * c) ? ((a > b) ? b : a) : dist;
}

/*计算两个方向角的角度差的绝对值*/
unsigned short direction_sub(unsigned short dir1, unsigned short dir2)
{
	short angle = abs(dir1 - dir2);

	return (angle <= 180) ? angle : (360 - angle);
}

/* 两直线的夹角，返回角度值，非弧度值 */
double IncludedAngle(long line1_x, long line1_y,long line2_x, long line2_y)

{
        double v = line1_x * line2_x + line1_y * line2_y;
        if (v == 0)	// 两直线垂直
                return 90;

        // 如果v=0, 则t为无穷大（输出则为inf. =infinity）
        // atan仍有返回值，为90度，并没有出错。
        double t = (line1_x*line2_y-line2_x*line1_y) / v;
        if (t < 0)
                t = 0 - t;

        return atan(t) * 180 / PI;
}

int is_in_line_range(double pt_lon, double pt_lat, double sl, double sb, double el, double eb)
{
        long line1_x = (sl - pt_lon) * 1000000;
        long line1_y = (sb - pt_lat) * 1000000;
        long line2_x = (el - pt_lon) * 1000000;
        long line2_y = (eb - pt_lat) * 1000000;

        double ang = IncludedAngle(line1_x, line1_y, line2_x, line2_y);

        if(ang < 100)
                return 0;
        else
                return -1;
}
