#pragma once
#include <stdint.h>
#include <cairo/cairo.h>

#define MAX_SG_NODE 2500

typedef struct _sg_node
{
	double  x;
	double  y;
	uint8_t r;
} sg_node_t;

typedef struct _sg_node_manage
{
	cairo_surface_t *surface;
	cairo_t         *cr;
	uint16_t        node_count;
	sg_node_t       node_buff[MAX_SG_NODE];
} sg_node_manage_t;

int sg_node_manage_init(sg_node_manage_t *p_manage, cairo_surface_t *surface, cairo_t *cr);

int sg_node_manage_add(sg_node_manage_t *p_manage, sg_node_t p_sg_node);

int sg_node_manage_draw(sg_node_manage_t *p_manage);

int sg_node_manage_destory(sg_node_manage_t *p_manage);

