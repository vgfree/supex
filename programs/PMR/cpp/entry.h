#pragma once

#include <stdio.h>
#include <stdint.h>

#include "major/smart_api.h"
#include "kv_cache.h"

#define GRID_EXTEND_LIMIT       g_locate_cfg->grid_extend_limit
#define PMR_MAX_DIST            g_locate_cfg->pmr_max_dist
#define PMR_MAX_SUB_DIR         g_locate_cfg->pmr_max_sub_dir

typedef struct locate_cfg_t
{
	char    seg_file_name[128];

	double  weight_no;
	double  weight_last_sam;
	double  weight_conn_direct;
	double  weight_conn_indirect;
	double  weight_byroad;
	double  weight_neaest;
	double  weight_min_sub;
	double  weight_speed;
	double  weight_alt;
	double  weight_indirect_in_node;
	double  grid_extend_limit;
	short   pmr_max_dist;
	short   pmr_max_sub_dir;
	short   track_back_deep;
	short   weight_dist_limit;
	short   weight_dir_limit;
	short   node_range_dist;
} locate_cfg_t;

extern kv_cache         *g_kv_cache;
extern locate_cfg_t     *g_locate_cfg;

void entry_init(void);

locate_cfg_t *locate_cfg_init(char *p_name);

