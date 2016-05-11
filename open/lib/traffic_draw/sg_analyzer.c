#include <stdlib.h>
#include <string.h>
#include "sg_analyzer.h"
#include "gps_mapping.h"

#define HIGHWAY 10
static int find_towway_road(sg_line_manage_t *line, sg_line_t *temp_line, int factor)
{
        int ret = sg_line_manage_find(line, temp_line);
        if(ret < 0)
                return -1;
        else if(ret == 1) {
                temp_line->x1 += factor;
                temp_line->x2 += factor;
                temp_line->y1 += factor;
                temp_line->y2 += factor;
                temp_line->reverse = 1;
        }
        return 0;
}

static void find_highway_road(sg_line_t *temp_line, int factor)
{
        if(temp_line->rt == HIGHWAY) {
                temp_line->x1 += factor;
                temp_line->x2 += factor;
                temp_line->y1 += factor;
                temp_line->y2 += factor;
                temp_line->width += 1;
        }
}

static void transform(sg_png_manag_t *gps,  sg_line_manage_t *line, sg_node_manage_t *node)
{
        sg_maxandmin_t set;
        int factor = gps->weight / 200;
        factor = (factor > 0) ? factor : 1;

        int ret = get_spacing(gps, &set, node);
        //int num = get_node(gps, node, factor+1);
        //memset(line, 0, sizeof(sg_line_manage_t));
        //if(ret<0 || num<0 || !set.L_space || !set.B_space) {
        if(ret<0 || !set.L_space || !set.B_space) {
                printf("transform error ret:%d L:%lf B:%lf\n", ret,set.L_space, set.B_space);
                //printf("transform error ret:%d L:%lf B:%lf num:%d", ret,set.L_space, set.B_space, num);
                return;
        }

        int alpha = gps->weight / set.L_space;
        int beta  = gps->height / set.B_space;

        sg_info_t *p = gps->cur_sg->next;//FIXME
        sg_line_t temp_line;
        while(p) {
                temp_line.x1 = (p->SL-set.MinL)*alpha;
                temp_line.y1 = gps->height - (p->SB-set.MinB)*beta;
                temp_line.x2 = (p->EL-set.MinL)*alpha;
                temp_line.y2 = gps->height - (p->EB-set.MinB)*beta;
                temp_line.rt = p->rt;
                //printf("> %d %d\n", temp_line.rt, p->rt);
                memcpy(temp_line.name, p->name, sizeof(p->name));

                int ret = find_towway_road(line, &temp_line, factor);
                if(ret < 0)
                        continue;
                find_highway_road(&temp_line, factor);
                temp_line.tt = p->tt;
                temp_line.width = factor;
                temp_line.rrid = p->rrid;
                temp_line.sgid = p->sgid;
                memcpy(temp_line.name, p->name, sizeof(p->name));
                sg_line_manage_add(line, &temp_line);
                memset(&temp_line, 0, sizeof(sg_line_t));
                p = p->next;
        }
        //sg_line_manage_print(line);

        int i;
        for(i = 0; i < ret; i++) {
                node->node_buff[i].x = (node->node_buff[i].x - set.MinL)*alpha; 
                node->node_buff[i].y = gps->height - (node->node_buff[i].y - set.MinB)*beta; 
        }
}

void gps_adaptation_topng(sg_png_manag_t *gps, sg_line_manage_t *line, sg_node_manage_t *node)
{
       transform(gps, line, node);
}
