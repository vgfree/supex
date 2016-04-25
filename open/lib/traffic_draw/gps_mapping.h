#pragma once
#include "sg_node.h"
#include "sg_info.h"
typedef struct _point
{
        double x, y;
} Point;

typedef struct _sg_maxandmin
{
        double MinL;
        double MinB;
        double MaxL;
        double MaxB;
        double L_space;
        double B_space;
} sg_maxandmin_t;

int get_spacing(sg_png_manag_t *sg, sg_maxandmin_t *ret);
int get_node(sg_png_manag_t *sg, sg_node_manage_t *node_t, int r);
