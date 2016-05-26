#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "major/alive_api.h"
#include "alive_cpp_api.h"
#include "load_alive_cfg.h"



struct alive_cfg_list   g_alive_cfg_list = {};



static void alive_entry_init(void)
{
	if (!kvpool_init()) {
		exit(EXIT_FAILURE);
	}
}



int main(int argc, char **argv)
{
	bool ok = false;

	// ---> init alive
	load_supex_args(&g_alive_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	ok = load_alive_cfg_file(&g_alive_cfg_list.file_info, g_alive_cfg_list.argv_info.conf_name);

	if (!ok) {
		exit(EXIT_FAILURE);
	}

	g_alive_cfg_list.func_info[UPSTREAM_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_alive_cfg_list.func_info[UPSTREAM_FUNC_ORDER].func = (TASK_VMS_FCB)alive_vms_call;

	g_alive_cfg_list.entry_init = alive_entry_init;


	alive_mount(&g_alive_cfg_list);

	alive_start();

	return 0;
}

