#include <stdio.h>
#include <stdlib.h>
#include "map_topo.h"
#include "topo_file.h"

int main()
{
	char                    *p_cfg_file = "topo_conf.json";
	topo_cfg_mysql_t        mysql_cfg;

	map_topo_init(p_cfg_file);
	map_topo_load();

	// 直接相连
	// uint32_t rrid1 = 1215795;
	// uint8_t sgid1 = 1;
	// uint32_t rrid2 = 1220349;
	// uint8_t sgid2 = 1;

	// 间接相连
	uint32_t        rrid1 = 1195463;
	uint8_t         sgid1 = 3;
	uint32_t        rrid2 = 1215753;
	uint8_t         sgid2 = 5;

	topo_node_t     node;
	int             ret = map_topo_isconnect(rrid1, sgid1, rrid2, sgid2, 2, &node);

	printf("map topo ret:%d\n", ret);

	if (ret == 2) {
		printf("lon1:%f,lat1:%f,lon2:%f,lat2:%f\n", node.L1, node.B1, node.L2, node.B2);
	}

	return 0;
}

