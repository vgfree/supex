#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "load_cfg.h"
#include "major/smart_api.h"
#include "entry.h"

#include "smart_evcoro_cpp_api.h"

static struct smart_cfg_list g_cfg_list = {};

int main(int argc, char **argv)
{
	load_supex_args(&g_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	load_cfg_file(&g_cfg_list.file_info, g_cfg_list.argv_info.conf_name);

	g_cfg_list.func_info[HMGET_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_cfg_list.func_info[HMGET_FUNC_ORDER].func = (TASK_VMS_FCB)api_hmget;
	g_cfg_list.func_info[HGETALL_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_cfg_list.func_info[HGETALL_FUNC_ORDER].func = (TASK_VMS_FCB)api_hgetall;
	g_cfg_list.entry_init = entry_init;

	smart_mount(&g_cfg_list);
	smart_start();
	return 0;
}

