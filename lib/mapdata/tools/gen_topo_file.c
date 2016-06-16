#include <stdio.h>
#include "map_topo.h"

int main()
{
	char                    *p_cfg_file = "topo_conf.json";
	topo_cfg_mysql_t        mysql_cfg;

	topo_cfg_init_mysql(p_cfg_file, &mysql_cfg);
	topo_file_gen(&mysql_cfg);

	return 0;
}

