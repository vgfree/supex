#pragma once
#include "topo_file.h"
#include "topo_cfg.h"
#include "topo_info.h"
#include <stdint.h>

int map_topo_init(char *p_file_name);

int map_topo_load();

int map_topo_isconnect(uint32_t rrid1, uint8_t sgid1, uint32_t rrid2, uint8_t sgid2, uint8_t deep, topo_node_t *p_node);

void map_topo_destory();

