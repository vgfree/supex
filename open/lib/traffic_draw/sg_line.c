#include <stdlib.h>
#include <string.h>
#include "sg_line.h"
#include "utils_draw.h"

int sg_line_manage_init(sg_line_manage_t *p_manage, cairo_surface_t *surface, cairo_t *cr)
{
	if (!p_manage) {
		return -1;
	}

	memset(p_manage, 0, sizeof(sg_line_manage_t));
	p_manage->surface = surface;
	p_manage->cr = cr;
	return 0;
}

int sg_line_manage_add(sg_line_manage_t *p_manage, sg_line_t *p_sg_line)
{
	if (p_manage->line_count + 1 > MAX_SG_LINE) {
		printf("line count large %d\n", p_manage->line_count);
		return -1;
	}

	p_manage->line_buff[p_manage->line_count] = *p_sg_line;
	p_manage->line_count++;

	return 0;
}

int sg_line_manage_find(sg_line_manage_t *p_manage, sg_line_t *p_sg_line)
{
	if (!p_manage || !p_sg_line) {
		printf("sg_line_manage_find error nil\n");
		return -1;
	}

	int i;

	for (i = 0; i < p_manage->line_count; i++) {
		if ((p_sg_line->rrid == p_manage->line_buff[i].rrid) \
			&& (p_sg_line->sgid == p_manage->line_buff[i].sgid)) {
			printf("same line..\n");
			return -2;
		}

		if ((p_sg_line->x1 == p_manage->line_buff[i].x2)	\
			&& (p_sg_line->x2 == p_manage->line_buff[i].x1)	\
			&& (p_sg_line->y1 == p_manage->line_buff[i].y2)	\
			&& (p_sg_line->y2 == p_manage->line_buff[i].y1)) {
			// printf("two-way %s\n", p_sg_line->name);
			// printf("find two-way road\n");
			return 1;
		}
	}

	return 0;
}

int sg_line_manage_print(sg_line_manage_t *p_manage)
{
	if (!p_manage) {
		printf("sg_line_manage_list error nil\n");
		return -1;
	}

	int i;

	for (i = 0; i < p_manage->line_count; i++) {
		printf("line.. %ld %ld %ld %ld\n", p_manage->line_buff[i].x1, p_manage->line_buff[i].y1, \
			p_manage->line_buff[i].x2, p_manage->line_buff[i].y2);
	}

	return 0;
}

int sg_line_manage_draw(sg_line_manage_t *p_manage)
{
	if (p_manage->line_count <= 0) {
		printf("draw line error:%d\n", p_manage->line_count);
		return -1;
	}

	int i;

	for (i = 0; i < p_manage->line_count; i++) {
		switch (p_manage->line_buff[i].tt)
		{
			case 1:
				traffic_path_green(p_manage->cr, p_manage->line_buff[i].x1, p_manage->line_buff[i].y1, p_manage->line_buff[i].x2, p_manage->line_buff[i].y2, p_manage->line_buff[i].width);
				break;

			case 2:
				traffic_path_yellow(p_manage->cr, p_manage->line_buff[i].x1, p_manage->line_buff[i].y1, p_manage->line_buff[i].x2, p_manage->line_buff[i].y2, p_manage->line_buff[i].width);
				break;

			case 3:
				traffic_path_red(p_manage->cr, p_manage->line_buff[i].x1, p_manage->line_buff[i].y1, p_manage->line_buff[i].x2, p_manage->line_buff[i].y2, p_manage->line_buff[i].width);
				break;

			case 5:
				traffic_path_brightred(p_manage->cr, p_manage->line_buff[i].x1, p_manage->line_buff[i].y1, p_manage->line_buff[i].x2, p_manage->line_buff[i].y2, p_manage->line_buff[i].width);
				break;

			default:
				traffic_path_gray(p_manage->cr, p_manage->line_buff[i].x1, p_manage->line_buff[i].y1, p_manage->line_buff[i].x2, p_manage->line_buff[i].y2, p_manage->line_buff[i].width);
				break;
		}
	}

	return 0;
}

int sg_line_manage_destory(sg_line_manage_t *p_manage)
{
	return 0;
}

