#pragma once
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <cairo-svg.h>

void traffic_path_bule(cairo_t *cr, int sx, int sy, int ex, int ey, int width);

void traffic_path_red(cairo_t *cr, int sx, int sy, int ex, int ey, int width);

void traffic_path_yellow(cairo_t *cr, int sx, int sy, int ex, int ey, int width);

void traffic_path_green(cairo_t *cr, int sx, int sy, int ex, int ey, int width);

void traffic_path_gray(cairo_t *cr, int sx, int sy, int ex, int ey, int width);

void traffic_path_brightred(cairo_t *cr, int sx, int sy, int ex, int ey, int width);

void show_roadname(cairo_t *cr, char *name, int x, int y, int size, double radians);

void traffic_node(cairo_t *cr, int x, int y, int r);

void traffic_rotate(cairo_t *cr, int alpha);

void traffic_scale(cairo_t *cr, double fx, double fy);

void traffic_translate(cairo_t *cr, double tx, double ty);

void show_nomalname(cairo_t *cr, char *name, int x, int y, int size);

