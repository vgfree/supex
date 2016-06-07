#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "entry.h"
#include "base/same_kill.h"
#include "load_swift_cfg.h"
// #include "luakvutils.h"
#ifdef GOBY
  #include "map_poi.h"
#endif
char *poi_file_name = NULL;
void entry_init(void)
{
#ifdef GOBY
	if (0 != mappoi_load()) {
		printf("ERROR , mappoi_load\n");
		exit(EXIT_FAILURE);
	}
#endif

	if (!kvpool_init()) {
		exit(EXIT_FAILURE);
	}
}

