#pragma once
#include <stdint.h>
#include <cairo/cairo.h>
#include <stdio.h>
#include "sg_line.h"
#include "sg_node.h"

typedef struct _sg_info {
        uint32_t rrid;
        uint16_t sgid;
        double SL;//起点
        double SB;
        double EL;
        double EB;
        char name[128];
        uint8_t tt;
        uint8_t rt;
        struct _sg_info *next;
}sg_info_t;

typedef struct _sg_png_manag {
        cairo_surface_t *surface;
        cairo_t* cr;
        char file_path[100];
        char file_name[50];
        uint16_t weight;
        uint16_t height;
        sg_line_manage_t line_manage;
        sg_node_manage_t node_manage;
        double lon;//当前gps信息 
        double lat;//lat
        short dir;//lon
        unsigned short sg_count;//路段数目
        sg_info_t *cur_sg;//单链表
} sg_png_manag_t;

int sg_png_manage_init(sg_png_manag_t *p_manage, char *p_path, char *p_name, uint16_t weight, uint16_t height);

void initList(sg_info_t **L);

void  ClearList(sg_info_t **L);

int sg_png_manage_add(sg_png_manag_t *p_manage, sg_info_t *sg_info);

int sg_png_manage_draw(sg_png_manag_t *p_manage);

void sg_png_manage_destory(sg_png_manag_t *p_manage);
