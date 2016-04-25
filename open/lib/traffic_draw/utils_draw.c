#include <math.h>
#include <string.h>

#include "utils_draw.h"

#define pen_radius 2
#define arc_r   4
#define TRANSPARENCY 0.5
#if 0
#define WIDTH  800
#define HEIGHT 600
#define STRIDE WIDTH*4
#define MAX_COORDS 1024

static void draw_init( cairo_t *cr )
{
        cairo_set_source_rgb (cr, 1,1,1);
        cairo_paint (cr);
}
#endif
void traffic_path_bule(cairo_t *cr, int sx, int sy, int ex, int ey, int width)
{
        cairo_set_line_width (cr, width);
        cairo_set_source_rgba (cr, .3, .42, .69, TRANSPARENCY);
        cairo_move_to (cr, sx, sy);
        cairo_line_to (cr, ex, ey);
        cairo_stroke(cr);
}

void traffic_path_red(cairo_t *cr, int sx, int sy, int ex, int ey, int width)
{
        cairo_set_line_width (cr, width);
        cairo_set_source_rgba (cr, 1, 0, 0, TRANSPARENCY);
        cairo_move_to (cr, sx, sy);
        cairo_line_to (cr, ex, ey);
        cairo_stroke(cr);
}

void traffic_path_brightred(cairo_t *cr, int sx, int sy, int ex, int ey, int width)
{
        cairo_set_line_width (cr, width);
        cairo_set_source_rgba (cr, 122/255.0, 7/255.0, 16/255.0, TRANSPARENCY);
        cairo_move_to (cr, sx, sy);
        cairo_line_to (cr, ex, ey);
        cairo_stroke(cr);
}

void traffic_path_gray(cairo_t *cr, int sx, int sy, int ex, int ey, int width)
{
        cairo_set_line_width (cr, width);
        cairo_set_source_rgba (cr, 153/255.0, 163/255.0, 164/255.0, TRANSPARENCY);
        cairo_move_to (cr, sx, sy);
        cairo_line_to (cr, ex, ey);
        cairo_stroke(cr);
}

void traffic_path_green(cairo_t *cr, int sx, int sy, int ex, int ey, int width)
{
        cairo_set_line_width (cr, width);
        cairo_set_source_rgba (cr, 30/255.0, 132/255.0, 73/255.0, TRANSPARENCY);
        cairo_move_to (cr, sx, sy);
        cairo_line_to (cr, ex, ey);
        cairo_stroke(cr);
}

void traffic_path_yellow(cairo_t *cr, int sx, int sy, int ex, int ey, int width)
{
        cairo_set_line_width (cr, width);
        cairo_set_source_rgba (cr, 229/255.0, 243/255.0, 16/255.0, TRANSPARENCY);
        cairo_move_to (cr, sx, sy);
        cairo_line_to (cr, ex, ey);
        cairo_stroke(cr);
}

void show_nomalname(cairo_t *cr, char *name, int x, int y, int size)
{
        cairo_set_source_rgba (cr, 30/255.0, 132/255.0, 73/255.0, 1);
        cairo_select_font_face (cr, "WenQuanYi Zen Hei Mono", CAIRO_FONT_SLANT_NORMAL,
                        CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size (cr, size);
        cairo_move_to (cr, x, y);
        cairo_show_text (cr, name);
}

void show_roadname( cairo_t *cr, char *name, int x, int y, int size, double radians)
{
        cairo_save(cr);
        cairo_text_extents_t textents;
        cairo_font_extents_t fextents;
        cairo_set_source_rgba (cr, 30/255.0, 132/255.0, 73/255.0, 1);
        cairo_select_font_face (cr, "WenQuanYi Zen Hei Mono", CAIRO_FONT_SLANT_NORMAL,
                        CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size (cr, size);
        cairo_font_extents(cr, &fextents);
        cairo_text_extents(cr, name, &textents);
//printf("font ascent:%lf descent:%lf height:%lf max_x_advance:%lf max_y_advance:%lf\n", fextents.ascent, \
                fextents.descent, fextents.height, fextents.max_x_advance, fextents.max_y_advance);
//printf("text x_bearing:%lf y_bearing:%lf width:%f height:%lf x_advance:%lf y_advance:%lf\n", textents.x_bearing, \
                textents.y_bearing, textents.width, textents.height, textents.x_advance, textents.y_advance);
        double nx, ny;
        nx = -textents.width/2.0;
        ny = fextents.height/2;
        cairo_translate(cr, x, y);
        cairo_rotate (cr, radians);
        cairo_translate(cr, nx, ny);
        cairo_move_to (cr, 0, 0);
        cairo_show_text (cr, name);
        cairo_restore(cr);
#if 0
        cairo_move_to (cr, 70.0, 165.0);
        cairo_text_path (cr, "void");
        cairo_set_source_rgb (cr, 0.5, 0.5, 1);
        cairo_fill_preserve (cr);
        cairo_set_source_rgb (cr, 0, 0, 0);
        cairo_set_line_width (cr, 2.56);
        cairo_stroke (cr);

        /* draw helping lines */
        cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
        cairo_arc (cr, 10.0, 135.0, 5.12, 0, 2*M_PI);
        cairo_close_path (cr);
        cairo_arc (cr, 70.0, 165.0, 5.12, 0, 2*M_PI);
        cairo_fill (cr); 
#endif
}

void traffic_node(cairo_t *cr, int x, int y, int r)
{
        cairo_set_line_width (cr, pen_radius*.5);

        /* Now we will draw the fancy circle around the "R" */
        /* NOTE: The angles are in radians */
        cairo_move_to (cr, x, y);
        double angle1 = 0 * (M_PI/180.0);  
        double angle2 = 360 * (M_PI/180.0);

        /* We draw a large black circle */
        cairo_set_source_rgba (cr, 0, 0, 0, 1);
        cairo_arc (cr, x, y, r, angle1, angle2);
        cairo_stroke (cr);

        /* We draw a smaller white circle centered on it */
        cairo_set_source_rgba (cr, 1, 1, 1, 1);
        cairo_arc (cr, x, y, r, angle1, angle2);
        /* We use the fill operator to fill in the circle! */
        cairo_fill (cr);
#if 0
        /* We are going to draw the letter "R" with black pen*/

        cairo_move_to (cr, 695,212); /* Bottom left of text at point */
        cairo_set_source_rgba (cr, 0, 0, 0, 1);
        cairo_select_font_face (cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                        CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size (cr, 40);
        cairo_show_text (cr, "S");
#endif
        /* We stroke everything we have just done 
           to actually draw it...
           */ 
        cairo_stroke (cr);
}

void traffic_rotate(cairo_t *cr, int alpha)
{
        cairo_matrix_t ma;
        cairo_get_matrix(cr, &ma);
        printf("ma before %lf %lf %lf %lf %lf %lf\n", ma.xx, ma.xy, ma.x0, ma.yx, ma.yy, ma.y0);
        //cairo_matrix_t ma2;

        cairo_rotate (cr, alpha * M_PI / 180);
        cairo_get_matrix(cr, &ma);
        printf("ma after %lf %lf %lf %lf %lf %lf\n", ma.xx, ma.xy, ma.x0, ma.yx, ma.yy, ma.y0);
}

void traffic_translate(cairo_t *cr, double tx, double ty)
{
        cairo_translate (cr, tx, ty);
}

void traffic_scale(cairo_t *cr, double fx, double fy)
{
        cairo_scale (cr, fx, fy);
}

static void save_context( cairo_t *cr )
{
        cairo_save(cr);
}

static void restore_context( cairo_t *cr )
{
        cairo_restore(cr);
}
#if 0
static void adjust_png( cairo_t *cr )
{

        cairo_translate(cr, 400, 300);
        cairo_rotate (cr, 15 * M_PI / 180);
        cairo_scale (cr, 2, 1.0);
}

/* Apply our path to the surface specified */
static void draw (cairo_surface_t *surface)
{
        cairo_t *cr;
        cr = cairo_create (surface);
        adjust_png(cr);
        draw_init (cr);
        traffic_path (cr);
        traffic_node (cr);
        show_roadname(cr, "hello", 10, 135, 90);
        cairo_destroy (cr);
}

/* We will draw our path on multiple surfaces to demonstrate 
   some of the various cairo backend
   */
        int
main (int    argc,
                char **argv)
{
        cairo_surface_t *surface;

        /* Image backend */
        surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
                        WIDTH, HEIGHT);
        draw (surface);
        cairo_surface_write_to_png (surface, "t4.png");
        cairo_surface_destroy (surface);
        return 0;
}
#endif
