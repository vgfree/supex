#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../map_grid.h"
#include "../pmr_cfg.h"

int main()
{
	map_grid_load_cfg       grid_cfg;
	map_line_load_cfg       line_cfg;

	pmr_load_cfg_mysql("./PMR_conf.json", &line_cfg, &grid_cfg);
	map_grid_load("map_grid_count", "map_grid_line");

	map_grid_destory();

	return 0;
}

