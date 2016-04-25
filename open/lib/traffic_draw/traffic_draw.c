#include "traffic_draw.h"
#include "sg_analyzer.h"
#include "sg_line.h"
#include "sg_node.h"
#include "loadgps.h"
#include "sg_adjust.h"
#include "sg_text.h"

#include <time.h>
#include <sys/time.h> 

int traffic_draw_start(sg_png_manag_t *p_manage, char *p_path, char *p_name, uint16_t weight, uint16_t height)
{
        struct  timeval  start;
        struct  timeval  end;
        unsigned long timer;
        //gettimeofday(&start,NULL);
        if(sg_png_manage_init(p_manage, p_path, p_name, weight, height) < 0)
                goto error;
        //printf("-------load gps before %d\n", p_manage->sg_count);
        //loadgps_form_tablefile("sgt", "surrounding_road", p_manage);
        //printf("-------load gps end %d\n", p_manage->sg_count);
        //gettimeofday(&end,NULL);
        //timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        //printf("timer load = %ld us\n",timer);

        sg_line_manage_t line;
        if(sg_line_manage_init(&line, p_manage->surface, p_manage->cr) < 0)
                goto error;
        sg_node_manage_t node;
        if(sg_node_manage_init(&node, p_manage->surface, p_manage->cr) < 0)
                goto error;

        gettimeofday(&start,NULL);
        gps_adaptation_topng(p_manage, &line, &node);
        //adjust_png(p_manage->weight, p_manage->height, 360 - p_manage->dir, &line, &node);
        sg_text_manage_t p_text;
        if (sg_text_manage_init(&p_text, p_manage->surface, p_manage->cr) < 0)
                goto error;

        sg_text_position(&line, &p_text);
        gettimeofday(&end,NULL);
        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        printf("timer prepare = %ld us\n",timer);

        gettimeofday(&start,NULL);

        if(sg_line_manage_draw(&line) < 0)
                goto error;
        if(sg_node_manage_draw(&node) < 0)
                goto error;

        if(sg_text_manage_draw(&p_text) < 0)
                goto error;

        sg_text_manage_destroy(&p_text);
        sg_png_manage_destory(p_manage);
        gettimeofday(&end,NULL);
        timer = 1000000 * (end.tv_sec-start.tv_sec)+ end.tv_usec-start.tv_usec;
        printf("timer draw = %ld us\n",timer);
        ClearList(&(p_manage->cur_sg));
        return 0;
error:
        sg_text_manage_destroy(&p_text);
        sg_png_manage_destory(p_manage);
        ClearList(&(p_manage->cur_sg));
        return -1;
}
