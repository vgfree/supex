#include <stdlib.h>
#include <string.h>

#include "utils_draw.h"
#include "sg_node.h"

int sg_node_manage_init(sg_node_manage_t *p_manage, cairo_surface_t *surface, cairo_t *cr)
{
	if (!p_manage) {
		return -1;
	}

	memset(p_manage, 0, sizeof(sg_node_manage_t));
	p_manage->surface = surface;
	p_manage->cr = cr;
	return 0;
}

int sg_node_manage_add(sg_node_manage_t *p_manage, sg_node_t p_sg_node)
{
	if (p_manage->node_count + 1 > MAX_SG_NODE) {
		// printf("node_count large :%d\n", p_manage->node_count);
		return -1;
	}

	p_manage->node_buff[p_manage->node_count] = p_sg_node;
	p_manage->node_count++;
	return p_manage->node_count;
}

int sg_node_manage_draw(sg_node_manage_t *p_manage)
{
	if (p_manage->node_count <= 0) {
		printf("draw node error: %d\n", p_manage->node_count);
		return -1;
	}

	int i;

	for (i = 0; i < p_manage->node_count; i++) {
		traffic_node(p_manage->cr, p_manage->node_buff[i].x, p_manage->node_buff[i].y, p_manage->node_buff[i].r);
	}

	return 0;
}

int sg_node_manage_destory(sg_node_manage_t *p_manage)
{
	return 0;
}

