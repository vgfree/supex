#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "map_grid_file.h"

int main()
{
	char    *config_file = "PMR_conf.json";
	char    *grid_count_file = "map_grid_count.data";
	char    *grid_line_file = "map_grid_line.data";

	map_grid_load_cfg       g_grid_cfg;
	map_line_load_cfg       g_line_cfg;

	assert(pmr_load_cfg_mysql(config_file, &g_line_cfg, &g_grid_cfg) == 0);

	map_grid_gen_file(grid_count_file, grid_line_file, &g_grid_cfg);

	//	map_grid_manager manage;
	//	map_grid_load_file(grid_count_file, grid_line_file, &manage, 10000);
	//	map_grid_manage_destory(&manage);

	return 0;
}

