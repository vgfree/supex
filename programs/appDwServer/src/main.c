#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "load_swift_cfg.h"
#include "major/swift_api.h"

#include "swift_lua_api.h"

struct swift_cfg_list g_swift_cfg_list = {};

void entry_init(void)
{
	if (!kvpool_init()) {
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	load_supex_args(&g_swift_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	load_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_call;

	g_swift_cfg_list.entry_init = entry_init;


	swift_mount(&g_swift_cfg_list);
	swift_start();
	return 0;
}

