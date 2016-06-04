#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "load_cfg.h"
#include "major/smart_api.h"

#include "smart_evcoro_lua_api.h"

struct smart_cfg_list g_smart_cfg_list = {};

static void entry_init(void)
{
	if (!kvpool_init()) {
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	load_supex_args(&g_smart_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	load_cfg_file(&g_smart_cfg_list.file_info, g_smart_cfg_list.argv_info.conf_name);

	g_smart_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_smart_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_VMS_FCB)smart_vms_call;
	g_smart_cfg_list.func_info[FETCH_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_smart_cfg_list.func_info[FETCH_FUNC_ORDER].func = (TASK_VMS_FCB)smart_vms_gain;
	g_smart_cfg_list.func_info[MERGE_FUNC_ORDER].type = BIT8_TASK_TYPE_WHOLE;
	g_smart_cfg_list.func_info[MERGE_FUNC_ORDER].func = (TASK_VMS_FCB)smart_vms_sync;
	g_smart_cfg_list.func_info[CUSTOM_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_smart_cfg_list.func_info[CUSTOM_FUNC_ORDER].func = (TASK_VMS_FCB)smart_vms_exec;

	g_smart_cfg_list.entry_init = entry_init;

	g_smart_cfg_list.vmsys_init = smart_vms_init;
	g_smart_cfg_list.vmsys_exit = smart_vms_exit;
	g_smart_cfg_list.vmsys_cntl = smart_vms_cntl;
	g_smart_cfg_list.vmsys_rfsh = smart_vms_rfsh;

	smart_mount(&g_smart_cfg_list);
	smart_start();
	return 0;
}

