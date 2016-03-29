#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "map_poi_file.h"
#include "poi_cfg.h"

int main()
{
	char                    *config_file = "POI_conf.json";
	mappoi_cfg_mysql        cfg;

	if (mappoi_load_cfg_mysql(&cfg, config_file) != 0) {
		printf("mappoi_load_cfg_data ERROR\n");
	}

	if (mappoi_gen_file(&cfg) != 0) {
		printf("mappoi_gen_file ERROR\n");
	}

	return 0;
}

