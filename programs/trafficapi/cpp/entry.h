#pragma once

#include <stdio.h>
#include <stdint.h>

#include "smart_api.h"

void entry_init(void);

int entry_cmd_roadrank(struct data_node *p_node);

int entry_cmd_front_traffic(struct data_node *p_node);

int entry_cmd_road_traffic(struct data_node *p_node);

