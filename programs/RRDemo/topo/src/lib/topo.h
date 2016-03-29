/*
 * topo_com.h
 *
 *  Created on: Jun 19, 2014
 *      Author: buyuanyuan
 */

#pragma once

#include "memory_bath.h"
#include "sql_api.h"

struct mem_list *topo_fetch_list(int type);

void topo_push_node(TP_NODE_OBJ *obj, uint64_t idx);

TP_NODE_OBJ *topo_pull_node(uint64_t idx);

void topo_push_line(TP_LINE_OBJ *obj, uint64_t idx);

TP_LINE_OBJ *topo_pull_line(uint64_t idx);

void topo_init(void);

void topo_make(void);

void topo_load(struct sql_info *info_node, struct sql_info *info_line);

