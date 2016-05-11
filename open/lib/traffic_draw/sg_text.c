#include "sg_text.h"
#include "utils_draw.h"

#include <stdio.h>                                                                                      
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#define FAC 8
static int replace_sgid(sg_text_t *sub_text, sg_line_t *line)
{
        if(line->sgid > sub_text->Esg) {
                sub_text->Esg = line->sgid;
                sub_text->Ex  = line->x2;
                sub_text->Ey  = line->y2;
                sub_text->dir = atan2(sub_text->Ey - sub_text->Sy, sub_text->Ex - sub_text->Sx);//FIXME
                return 1;
        }

        if(line->sgid < sub_text->Ssg) {
                sub_text->Ssg = line->sgid;
                sub_text->Sx  = line->x1;
                sub_text->Sy  = line->y1;
                sub_text->dir = atan2(sub_text->Ey - sub_text->Sy, sub_text->Ex - sub_text->Sx);//FIXME
                return 2;
        }
        return 0;
}

int sg_text_manage_init(sg_text_manage_t *p_manage, cairo_surface_t *surface, cairo_t *cr)
{
        if(!p_manage)
                return -1;

        memset(p_manage, 0, sizeof(sg_text_manage_t));
        p_manage->surface =  surface;
        p_manage->cr = cr;
        return 0;
}

int sg_text_position(sg_line_manage_t *p_line, sg_text_manage_t *p_text)
{
        if(p_line->line_count <= 0) {
                printf("calculate_text_position error! line count:%d\n", p_line->line_count);
                return -1;
        }                
        
        p_text->ht = create_hashtable(MAX_SG_TEXT,int,intptr_t);

        int i;           
        for(i = 0; i < p_line->line_count; i++) {
                if(p_line->line_buff[i].reverse == 1)
                        continue;

                if(hash_exists(p_text->ht, p_line->line_buff[i].rrid) == NOTEXISTS){
                        //printf("\tnot exists\n");
                        sg_text_t *sub_text = (sg_text_t*)malloc(sizeof(sg_text_t));
                        if(!sub_text)
                                return;

                        sub_text->Ssg = p_line->line_buff[i].sgid;
                        sub_text->Esg = p_line->line_buff[i].sgid;
                        sub_text->Sx  = p_line->line_buff[i].x1;
                        sub_text->Sy  = p_line->line_buff[i].y1;
                        sub_text->Ex  = p_line->line_buff[i].x2;
                        sub_text->Ey  = p_line->line_buff[i].y2;
                        sub_text->rt  = p_line->line_buff[i].rt;
                        sub_text->width  = p_line->line_buff[i].width;
                        sub_text->dir = atan2(sub_text->Ey - sub_text->Sy, sub_text->Ex - sub_text->Sx);//FIXME
                        memcpy(sub_text->name, p_line->line_buff[i].name, sizeof(p_line->line_buff[i].name));
                        hash_add(p_text->ht,p_line->line_buff[i].rrid,(intptr_t)sub_text);
                }
                else if (hash_exists(p_text->ht, p_line->line_buff[i].rrid) == EXISTS){
                        //printf("\tstill exists\n");
                        intptr_t ret;
                        hash_find(p_text->ht,p_line->line_buff[i].rrid,&ret);
                        sg_text_t *temp_text = (sg_text_t *)ret;
                        replace_sgid(temp_text, &p_line->line_buff[i]);
                }

        }
        return 0;
}

void sg_text_manage_print(sg_text_manage_t *p_text)
{
        if(!p_text)
                return;

        HashTable *ht = p_text->ht;
        intptr_t ivalue;
        sg_text_t *temp_text = NULL;
        for (reset(ht);isnotend(ht);next(ht)){
                int key1 = nkey(ht);
                ivalue = *(intptr_t*)value(ht);
                temp_text = (sg_text_t *)ivalue;
                if(temp_text) {
                        printf("hash:\t %s %lf %d %d\n", temp_text->name, temp_text->dir*180/3.1415926, temp_text->Sx, temp_text->Sy);
                }
        }
}

static inline void radian_transition(sg_text_t *text, int *x)
{
        if(text->dir < 0) {
                text->dir += M_PI;
                *x += text->width*FAC;
        }
        else {
                *x -= text->width*FAC;
                if(text->dir >= M_PI/4)
                        text->dir -= M_PI;
        }
}

int sg_text_manage_draw(sg_text_manage_t *p_text)
{
        if(!p_text)
                return -1;

        HashTable *ht = p_text->ht;
        intptr_t ivalue;
        sg_text_t *temp_text;
        int mx, my;
        for (reset(ht);isnotend(ht);next(ht)){
                int key1 = nkey(ht);
                ivalue = *(intptr_t*)value(ht);
                temp_text = (sg_text_t *)ivalue;

                if(temp_text) {
                        //高速与高架路名放置在起点
                        if(temp_text->rt != 0 || temp_text->rt != 10) {
                                mx = (temp_text->Sx+temp_text->Ex)/2;
                                my = (temp_text->Sy+temp_text->Ey)/2;
                        }
                        else {
                                mx = temp_text->Sx;
                                my = temp_text->Sy;
                        }

                        //printf("hash:\t %s %lf %d %d\n", temp_text->name, temp_text->dir*180/3.1415926, temp_text->Sx, temp_text->Sy);
                        radian_transition(temp_text, &mx);
                        //printf("rt: %d\n", temp_text->rt);
                        /*
                         *['0'] = "高速",
                         *['1'] = "国道",
                         *['2'] = "省道",
                         *['3'] = "县道",   
                         *['4'] = "乡道",
                         *['5'] = "村道",
                         *['7'] = "普通道路",
                         *['10'] = "城市快速路",
                         *['11'] = "城市主干道",
                         *['12'] = "城市次干道",
                         *['15'] = "步行街",
                         *['16'] = "内部道路",
                         * */
                        if(temp_text->rt % 10 < 3)
                                show_roadname(p_text->cr, temp_text->name, mx, my, 20, temp_text->dir);
                        //show_nomalname(p_text->cr, temp_text->name, mx, my, 20);
                }
        }
        return 0;
}

void sg_text_manage_destroy(sg_text_manage_t *p_text)
{
        if(!p_text)
                return;

        HashTable *ht = p_text->ht;
        intptr_t ivalue;
        sg_text_t *temp_text;
        for (reset(ht);isnotend(ht);next(ht)){
                int key1 = nkey(ht);
                ivalue = *(intptr_t*)value(ht);
                temp_text = (sg_text_t *)ivalue;
                if(temp_text)
                        free(temp_text);
        }
        if(ht)
                hash_free(ht);
        ht = NULL;
}
