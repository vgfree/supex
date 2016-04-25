#include "sg_adjust.h"
#include <stdlib.h>
#include <math.h>

#define PI 3.1415926535
#define RADIAN(angle) ((angle)*PI/180.0)

#define max(a, b) ((a) > (b) ? (a) : (b))

typedef struct _point_t
{       
        double x, y;
} POINT;

static void rotate_png(int srcW, int srcH, int angle, sg_line_manage_t *line, sg_node_manage_t *node)
{
        if(!line || srcW <= 0 || srcH <= 0 || angle <= 0)
                return;
        //边角
        POINT pLT,pRT,pLB,pRB;
        pLT.x = -srcW/2;pLT.y = srcH/2;
        pRT.x = srcW/2;pRT.y = srcH/2;
        pLB.x = -srcW/2;pLB.y = -srcH/2;
        pRB.x = srcW/2; pRB.y = -srcH/2;
        //旋转之后的坐标
        POINT pLTN,pRTN,pLBN,pRBN;
        double sina = sin(RADIAN(angle));
        double cosa = cos(RADIAN(angle));
        pLTN.x = pLT.x*cosa + pLT.y*sina;
        pLTN.y = -pLT.x*sina + pLT.y*cosa;
        pRTN.x = pRT.x*cosa + pRT.y*sina;
        pRTN.y = -pRT.x*sina + pRT.y*cosa;
        pLBN.x = pLB.x*cosa + pLB.y*sina;
        pLBN.y = -pLB.x*sina + pLB.y*cosa;
        pRBN.x = pRB.x*cosa + pRB.y*sina;
        pRBN.y = -pRB.x*sina + pRB.y*cosa;

        //旋转后图像宽和高
        int desWidth = max(abs(pRBN.x - pLTN.x),abs(pRTN.x - pLBN.x));
        int desHeight = max(abs(pRBN.y - pLTN.y),abs(pRTN.y - pLBN.y));

        printf("src %d %d des %d %d\n", srcW, srcH, desWidth, desHeight);

        int i;
        for(i = 0; i<line->line_count; i++) {
                sg_line_t temp_line = line->line_buff[i];
                //转换到以图像为中心的坐标系，并进行逆旋转
                int tX1 = (temp_line.x1 - desWidth / 2)*cos(RADIAN(360 - angle)) + (-temp_line.y1 + desHeight / 2)*sin(RADIAN(360 - angle));
                int tY1 = -(temp_line.x1 - desWidth / 2)*sin(RADIAN(360 - angle)) + (-temp_line.y1 + desHeight / 2)*cos(RADIAN(360 - angle));
                int tX2 = (temp_line.x2 - desWidth / 2)*cos(RADIAN(360 - angle)) + (-temp_line.y2 + desHeight / 2)*sin(RADIAN(360 - angle));
                int tY2 = -(temp_line.x2 - desWidth / 2)*sin(RADIAN(360 - angle)) + (-temp_line.y2 + desHeight / 2)*cos(RADIAN(360 - angle));
                //如果这个坐标不在原图像内，则不赋值
                /*if (tX1 > srcW / 2 || tX1 < -srcW / 2 || tY1 > srcH / 2 || tY1 < -srcH / 2 \
                        || tX2 > srcW / 2 || tX2 < -srcW / 2 || tY2 > srcH / 2 || tY2 < -srcH / 2)
                {
                        continue;
                }*/
                //再转换到原坐标系下
                int tXN1 = tX1 + srcW / 2; int tYN1 = abs(tY1 - srcH / 2);
                int tXN2 = tX2 + srcW / 2; int tYN2 = abs(tY2 - srcH / 2);
                //printf("---------------------%d\n", i);
                //printf("%d %d %d %d\n", line->line_buff[i].x1, line->line_buff[i].y1, line->line_buff[i].x2, line->line_buff[i].y2);
                line->line_buff[i].x1 = tXN1;
                line->line_buff[i].y1 = tYN1;
                line->line_buff[i].x2 = tXN2;
                line->line_buff[i].y2 = tYN2;
                //printf("%d %d %d %d\n", line->line_buff[i].x1, line->line_buff[i].y1, line->line_buff[i].x2, line->line_buff[i].y2);
        }

        for(i = 0; i<node->node_count; i++) {
                sg_node_t temp_node = node->node_buff[i];
                int tX1 = (temp_node.x - desWidth / 2)*cos(RADIAN(360 - angle)) + (-temp_node.y + desHeight / 2)*sin(RADIAN(360 - angle));
                int tY1 = -(temp_node.x - desWidth / 2)*sin(RADIAN(360 - angle)) + (-temp_node.y + desHeight / 2)*cos(RADIAN(360 - angle));
                int tXN1 = tX1 + srcW / 2; int tYN1 = abs(tY1 - srcH / 2);
                node->node_buff[i].x = tXN1;
                node->node_buff[i].y = tYN1;
        }
}

void adjust_png(int srcW, int srcH, int angle, sg_line_manage_t *line, sg_node_manage_t *node)
{
        /*
        traffic_translate(cr, 400, 300);
        cairo_matrix_t ma;
        cairo_get_matrix(cr, &ma);
        printf("ma before %lf %lf %lf %lf %lf %lf\n", ma.xx, ma.xy, ma.x0, ma.yx, ma.yy, ma.y0);
        traffic_scale(cr, 0.5, 1);
        cairo_get_matrix(cr, &ma);
        printf("ma after %lf %lf %lf %lf %lf %lf\n", ma.xx, ma.xy, ma.x0, ma.yx, ma.yy, ma.y0);
        traffic_rotate(cr, alpha);
        */
        rotate_png(srcW, srcH, angle, line, node);
}
