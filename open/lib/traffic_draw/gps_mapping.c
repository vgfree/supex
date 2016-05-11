#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "hashtable.h"
#include "gps_mapping.h"

//gps坐标 转换为 图像坐标经度
#define FACTOR 1

#define PI 3.1415926535
#define RADIAN(angle) ((angle)*PI/180.0)

static void rotate_gps(sg_info_t *line, short dir)
{
        line->SL *= FACTOR;
        line->SB *= FACTOR;
        line->EL *= FACTOR;
        line->EB *= FACTOR;
//printf("------------------------------------\n");
//printf("%lf %lf %lf %lf\n", line->SL, line->SB, line->EL, line->EB);
        double SL, SB, EL, EB;
        if(dir == -1 || dir == 0) {
                return;
        }

        double sina = sin(RADIAN(dir));
        double cosa = cos(RADIAN(dir));

        SL = line->SL*cosa + line->SB*sina;
        SB = -line->SL*sina + line->SB*cosa;
        EL = line->EL*cosa + line->EB*sina;
        EB = -line->EL*sina + line->EB*cosa;

        line->SL = SL;
        line->SB = SB;
        line->EL = EL;
        line->EB = EB;
//printf("%lf %lf %lf %lf\n", line->SL, line->SB, line->EL, line->EB);
}

static double determinant(double v1, double v2, double v3, double v4)  // 行列式
{
        return (v1*v4-v2*v3);
}

static bool intersect(sg_info_t aline, sg_info_t bline)
{
        Point aS, aE, bS, bE;
        aS.x = aline.SL, aS.y = aline.SB;
        aE.x = aline.EL, aE.y = aline.EB;
        bS.x = bline.SL, bS.y = bline.SB;
        bE.x = bline.EL, bE.y = bline.EB;
        double delta = determinant(aE.x-aS.x, bS.x-bE.x, aE.y-aS.y, bS.y-bE.y);
        if ( delta<=(1e-7) && delta>=-(1e-7) )  // delta=0，表示两线段重合或平行
        {
                return false;
        }
        double namenda = determinant(bS.x-aS.x, bS.x-bE.x, bS.y-aS.y, bS.y-bE.y) / delta;
        if ( namenda>1 || namenda<0 )
        {
                return false;
        }
        double miu = determinant(aE.x-aS.x, bS.x-aS.x, aE.y-aS.y, bS.y-aS.y) / delta;
        if ( miu>1 || miu<0 )
        {
                return false;
        }
        return true;
}
#if 0
char get_line_intersection(float p0_x, float p0_y, float p1_x, float p1_y, 
    float p2_x, float p2_y, float p3_x, float p3_y, float *i_x, float *i_y)
{
    float s1_x, s1_y, s2_x, s2_y;
    s1_x = p1_x - p0_x;     s1_y = p1_y - p0_y;
    s2_x = p3_x - p2_x;     s2_y = p3_y - p2_y;

    float s, t;
    s = (-s1_y * (p0_x - p2_x) + s1_x * (p0_y - p2_y)) / (-s2_x * s1_y + s1_x * s2_y);
    t = ( s2_x * (p0_y - p2_y) - s2_y * (p0_x - p2_x)) / (-s2_x * s1_y + s1_x * s2_y);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
    {
        // Collision detected
        if (i_x != NULL)
            *i_x = p0_x + (t * s1_x);
        if (i_y != NULL)
            *i_y = p0_y + (t * s1_y);
        return 1;
    }

    return 0; // No collision
}
#endif
static sg_node_t getIntersg_node(sg_info_t a, sg_info_t b) 
{
        //行列式求两条直线交点
        sg_node_t hp;
        double D = (a.EL-a.SL)*(b.SB-b.EB)
                -(b.EL-b.SL)*(a.SB-a.EB);
        double D1 = (b.SB*b.EL-b.SL*b.EB)
                *(a.EL-a.SL)
                -(a.SB*a.EL-a.SL*a.EB)
                *(b.EL-b.SL);
        double D2 = (a.SB*a.EL-a.SL*a.EB)
                *(b.SB-b.EB)
                -(b.SB*b.EL-b.SL*b.EB)
                *(a.SB-a.EB);
        hp.x = D1/D;
        hp.y = D2/D;
        return hp;
}

int get_spacing(sg_png_manag_t *sg, sg_maxandmin_t *ret, sg_node_manage_t *node_t)
{
        if(!sg || !ret || !sg->sg_count) {
                printf("list is null... sg_png_manag_t:%p sg_maxandmin_t:%p sg_count %d\n", sg, ret, sg->sg_count);
                return -1;
        }
        
        int factor = sg->weight / 200;  
        factor = (factor > 0) ? factor : 1;

        sg_info_t *p = sg->cur_sg->next;//FIXME
        double MinL, MinB, MaxL, MaxB;
        MinL = 200*FACTOR, MinB = 200*FACTOR, MaxL = -1, MaxB = -1;

        HashTable *ht = create_hashtable(100,char*,int);
        if(!ht)
                return -1;

        char keyS[20] = "";
        char keyE[20] = "";
        sg_node_t p_node;
        while(p) {
                //rotate_gps(p, sg->dir);
                if(p->SL < MinL)
                        MinL = p->SL;
                if(p->SB < MinB)
                        MinB = p->SB;
                if(p->EL < MinL)
                        MinL = p->EL;
                if(p->EB < MinB)
                        MinB = p->EB;
                if(p->SL > MaxL)
                        MaxL = p->SL;
                if(p->SB > MaxB)
                        MaxB = p->SB;
                if(p->EL > MaxL)
                        MaxL = p->EL;
                if(p->EB > MaxB)
                        MaxB = p->EB;

                sprintf(keyS, "%lf&%lf", p->SL, p->SB);
                sprintf(keyE, "%lf&%lf", p->EL, p->EB);
                if(hash_exists(ht, keyS) == NOTEXISTS) {
                        hash_add(ht, keyS, 1);
                }
                else if(hash_exists(ht, keyS) == EXISTS) {
                        p_node.x = p->SL;
                        p_node.y = p->SB;
                        p_node.r = factor+1;
                        sg_node_manage_add(node_t, p_node);
                }
                if(hash_exists(ht, keyE) == NOTEXISTS) {
                        hash_add(ht, keyE, 1);
                }
                else if(hash_exists(ht, keyE) == EXISTS) {
                        p_node.x = p->EL;
                        p_node.y = p->EB;
                        p_node.r = factor+1;
                        sg_node_manage_add(node_t, p_node);
                }
                p = p->next;
        }
        hash_free(ht);
        ret->MinL = MinL;
        ret->MinB = MinB;
        ret->MaxL = MaxL;
        ret->MaxB = MaxB;
        ret->L_space = MaxL - MinL;
        ret->B_space = MaxB - MinB;
        return node_t->node_count;
}
/*
 *获取路段交点，需在get_spacing后调用
 * */
int get_node(sg_png_manag_t *sg, sg_node_manage_t *node_t, int r)
{
        if(!sg || !node_t || !sg->sg_count) {
                printf("get node error sg_png_manag_t sg_node_manage_t is null sg_png_manag_t:%p sg_node_manage_t:%p sg_count：%d\n", sg, node_t, sg->sg_count);
                return -1;
        }
        bool has;
        sg_node_t p_node;
        sg_info_t *p, *q = sg->cur_sg->next;//FIXME
        p = q;
        
        while(p) {
                while(q) {
                        has = intersect(*p, *q);
                        if(has == true) {
                                p_node = getIntersg_node(*p, *q);
                                p_node.r = r;
                                sg_node_manage_add(node_t, p_node);
                        }
                        q = q->next;
                }
                //q = sg->cur_sg->next;
                p = p->next;
                q = p;
        }
        return node_t->node_count;
}
