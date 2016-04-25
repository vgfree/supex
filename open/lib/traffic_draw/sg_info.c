#include "sg_info.h"
#include <stdlib.h>
#include <string.h>


#define DEFWEIGHT 800
#define DEFHEIGHT 600

void initList(sg_info_t **L)
{
        *L=(sg_info_t*)malloc(sizeof(sg_info_t)); /* 产生头结点,并使L指向此头结点 */
        if(!(*L))
                return;
        (*L)->next=NULL;
}

void  ClearList(sg_info_t **L)
{ 
        sg_info_t *p,*q;
        p=(*L)->next;           /*  p指向第一个结点 */
        while(p)                /*  没到表尾 */
        {
                q=p->next;
                if(p)
                        free(p);
                p=q;
        }
        (*L)->next=NULL;        /* 头结点指针域为空 */
        if(*L)
                free(*L);
}

/* 操作结果：在L中第i个位置之前插入新的数据元素e，L的长度加1 */
void ListInsert(sg_info_t **L, int i, sg_info_t *node)
{       
        int j;
        sg_info_t *p, *s;
        p = *L;   
        j = 1;
        while (p && j < i)     /* 寻找第i个结点 */
        {
                p = p->next;
                ++j;
        } 
        if (!p || j > i) 
                return;   /* 第i个元素不存在 */
        s = (sg_info_t*)malloc(sizeof(sg_info_t));
        s->rrid = node->rrid;
        s->sgid = node->sgid;
        s->SB = node->SB;
        s->SL = node->SL;
        s->EB = node->EB;
        s->EL = node->EL;
        strcpy(s->name, node->name);
        s->tt = node->tt;
        s->next = p->next;
        p->next = s;
}

int sg_png_manage_init(sg_png_manag_t *p_manage, char *p_path, char *p_name, uint16_t weight, uint16_t height)
{
        if(!p_manage) {
                printf("list is null init error %p %p\n", p_manage, p_manage->cur_sg);
                return -1;
        }

        //memset(p_manage, 0, sizeof(sg_png_manag_t));
        strncpy(p_manage->file_path, p_path, sizeof(p_manage->file_path));
        strncpy(p_manage->file_name, p_name, sizeof(p_manage->file_name));

        if(weight > 0)
                p_manage->weight = weight;
        else    p_manage->weight = DEFWEIGHT;
        if(height > 0)
                p_manage->height = height;
        else    p_manage->height = DEFHEIGHT;

        /* 创建32位RGBA颜色格式的Cairo绘图环境，直接在Memory中渲染 */
        cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, p_manage->weight + 1, p_manage->height + 1);

        if(!surface) {
                printf("cairo_image_surface_create error\n");
                return -1;
        }

        cairo_t* cr = cairo_create(surface);

        if(!cr) {
                printf("cairo_create error\n");
                return -2;
        }

        p_manage->surface = surface;
        p_manage->cr = cr;
        //p_manage->dir = 90;

//初始化 链表
        //initList(&(p_manage->cur_sg));

        return 0;
}

int sg_png_manage_add(sg_png_manag_t *p_manage, sg_info_t *sg_info)
{
        p_manage->sg_count++;
//链表 插入
        ListInsert(&(p_manage->cur_sg), 1, sg_info);
        return 0;
}

int sg_png_manage_draw(sg_png_manag_t *p_manage)
{

        char full_name[156] = {0};
        sprintf(full_name, "%s/%s", p_manage->file_path, p_manage->file_name);
        /* 将Memory的渲染效果存储到图片中 */
        cairo_surface_write_to_png(p_manage->surface, full_name);
        return 0;
}

void sg_png_manage_destory(sg_png_manag_t *p_manage)
{
        /* 销毁并退出Cairo绘图环境 */
        cairo_destroy(p_manage->cr);
        sg_png_manage_draw(p_manage);
        cairo_surface_destroy (p_manage->surface);
}
