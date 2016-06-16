#pragma once
#include "sg_info.h"
#include <stdint.h>

/*
 *功能：周边路况做图入口函数
 *参数：p_manage:结构体
 *      p_path:  保存路径，如 "/home"
 *      p_name:  文件名，  如 "trri.png"
 *      图片宽度：         如 800
 *      图片高度：         如 600
 *返回值：
 *      -1 失败
 *      0  成功
 * */

int traffic_draw_start(sg_png_manag_t *p_manage, char *p_path, char *p_name, uint16_t weight, uint16_t height);

