#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "map_seg_file.h"

int main()
{
	char            *config_file = "SEG_conf.json";
	char            *seg_file = "map_seg.data";
	map_seg_cfg     sgid_cfg;

	assert(seg_load_cfg_mysql(config_file, &sgid_cfg) == 0);
	// printf("sgid_cfg.index_long=%zi\n", sgid_cfg.index_long);
	// printf("sgid_cfg.rrid_buf_long=%zi\n", sgid_cfg.rrid_buf_long);
	// printf("sgid_cfg.name_index_long=%zi\n", sgid_cfg.name_index_long);
	// printf("sgid_cfg.name_buf_long=%zi\n", sgid_cfg.name_buf_long);

	map_seg_file(seg_file, &sgid_cfg);
	// seg_file_load(seg_file) ;
	// map_line_manager_destory(&manage);

	return 0;
}

