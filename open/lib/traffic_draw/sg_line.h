#pragma once
#include <cairo/cairo.h>
#include <stdint.h>

#define SG_LINE_WEIGHT  5
#define MAX_SG_LINE     2500

typedef struct _sg_line
{
	uint8_t         tt;
	uint8_t         rt;
	int64_t         x1;
	int64_t         y1;
	int64_t         x2;
	int64_t         y2;
	uint8_t         width;
	uint32_t        rrid;
	uint16_t        sgid;
	short           reverse;
	char            name[128];
} sg_line_t;

typedef struct _sg_line_manage
{
	cairo_surface_t *surface;
	cairo_t         *cr;
	uint16_t        line_count;
	sg_line_t       line_buff[MAX_SG_LINE];
} sg_line_manage_t;

int sg_line_manage_init(sg_line_manage_t *p_manage, cairo_surface_t *surface, cairo_t *cr);

int sg_line_manage_add(sg_line_manage_t *p_manage, sg_line_t *p_sg_line);

int sg_line_manage_print(sg_line_manage_t *p_manage);

int sg_line_manage_find(sg_line_manage_t *p_manage, sg_line_t *p_sg_line);

int sg_line_manage_draw(sg_line_manage_t *p_manage);

int sg_line_manage_destory(sg_line_manage_t *p_manage);

