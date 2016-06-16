/*
 * common.h
 *
 *  Created on: Jan 27, 2016
 *  Author: shu
 */

#pragma once

#include <stdint.h>
#include <unistd.h>

#define MAX_TOPO_NODE_COUNT             100
#define MAX_TOPO_LINE_COUNT             100
#define MAX_ONE_NODE_OWN_LINE_COUNT     8

#define TYPE_TOPO_NODE                  0
#define TYPE_TOPO_LINE                  1

typedef struct tp_node_obj
{
	uint64_t                id;

	double                  longitude;
	double                  latitude;

	struct tp_line_obj      *hipt_line;	/*head import line*/
	struct tp_line_obj      *hept_line;	/*head export line*/
} TP_NODE_OBJ;

typedef struct tp_line_obj
{
	uint64_t                id;
	uint64_t                ban;		/*we can use bit to record the index which is ban when foreach the list*/
	double                  direction;	// 记录方向
	uint64_t                length;		// 记录道路长度
	int                     avegare_speed;	// 记录平均速度
	int                     limit_speed;	// 记录限速
	int                     color;		// 记录color
	float                   rank;		// 记录rank

	struct tp_node_obj      *from_node;
	struct tp_node_obj      *goto_node;

	struct tp_line_obj      *pfne_line;	/*prev from_node's export line*/
	struct tp_line_obj      *nfne_line;	/*next from_node's export line*/

	struct tp_line_obj      *pgni_line;	/*prev goto_node's import line*/
	struct tp_line_obj      *ngni_line;	/*next goto_node's import line*/
} TP_LINE_OBJ;

/* */
struct vehicle_obj
{
	double          longitude;
	double          latitude;
	long long       imei;
	int             magic;

	uint64_t        roadid;
	int             priority;
	int             speed;
	TP_LINE_OBJ     road;
};

