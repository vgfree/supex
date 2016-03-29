#include "map_poi.h"
#include "poi_cfg.h"
#include <stdio.h>
#include <unistd.h>

int main()
{
	char *cfg_file_name = "POI_conf.json";

	if (0 != mappoi_load_cfg(cfg_file_name)) {
		printf("ERROR , mappoi_load_cfg\n");
	}

	if (0 != mappoi_load()) {
		printf("ERROR , mappoi_load\n");
	}

	mappoi_iterator *p_it = mappoi_iterator_init(1231998, 1, 10);

	if (p_it == NULL) {
		printf("malloc error \n");
		return 0;
	}

	mappoi_poi_buf *out;

	while (mappoi_iterator_next(p_it, &out) == 0) {
		if (out != NULL) {
			int j = 0;

			for (j = 0; j < out->poi_idx; j++) {
				mappoi_poi *p_poi_j = out->poi + j;
				printf("poi_id:%d, poi_type:%d,x:%f,y:%f\n", p_poi_j->poi_id, p_poi_j->poi_type, p_poi_j->point_x, p_poi_j->point_y);
			}
		}
	}

	mappoi_iterator_destory(p_it);

	return 0;
}

