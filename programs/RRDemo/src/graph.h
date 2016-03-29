/**
 * Author       : chenzutao
 * Date         : 2016-01-07
 * Function     : graph.h
 *                graph data structure
 **/
#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned int uint_t;

typedef struct vertex_attr_s
{
	int     color;
	int     partition;
} vertex_attr_t;

typedef struct edge_attr_s
{
	int     speed;
	int     limit_speed;
	int     time;
	int     level;
	int     len;
	float   rank;
	float   weight;
} edge_attr_t;

typedef struct vertex_s
{
	uint_t          v_id;
	uint_t          out_edge;
	uint_t          in_edge;
	vertex_attr_t   v_attr;
} vertex_t;

typedef struct edge_s
{
	uint_t          e_id;
	uint_t          src_vertice;
	uint_t          dst_vertice;
	//        vertex_t src_vertex;
	//        vertex_t dst_vertex;
	edge_attr_t     edge_attr;
} edge_t;

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __GRAPH_H__ */

