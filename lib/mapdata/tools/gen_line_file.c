#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "map_line_file.h"

int main()
{
	char    *config_file = "PMR_conf.json";
	char    *line_file = "map_line.data";

	map_grid_load_cfg       g_grid_cfg;
	map_line_load_cfg       g_line_cfg;

	assert(pmr_load_cfg_mysql(config_file, &g_line_cfg, &g_grid_cfg) == 0);

	map_line_gen_file(line_file, &g_line_cfg);

	map_line_manager manage;
	map_line_load_file(line_file, &manage, 10000);
	map_line_manager_destory(&manage);

	return 0;
}

