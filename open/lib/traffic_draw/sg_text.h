#pragma once
#include "utils_draw.h"
#include "sg_line.h"
#include "hashtable.h"

#define MAX_SG_TEXT 250

typedef struct _sg_text_t {
        int Sx;
        int Sy;
        int Ex;
        int Ey;
        int Ssg;
        int Esg;
        short rt;
        uint8_t width;
        float dir;
        char name[128];
} sg_text_t;

typedef struct _sg_text_manage {
        cairo_surface_t *surface;
        cairo_t* cr;
        int text_count;
        //sg_text_t text_buff[MAX_SG_TEXT];
        HashTable *ht;
} sg_text_manage_t;

int sg_text_manage_init(sg_text_manage_t *p_manage, cairo_surface_t *surface, cairo_t *cr);
int sg_text_position(sg_line_manage_t *p_line, sg_text_manage_t *p_text);
void sg_text_manage_destroy(sg_text_manage_t *p_text);
int sg_text_manage_draw(sg_text_manage_t *p_text);
