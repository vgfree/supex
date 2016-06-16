#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "binheap.h"
#include "topo.h"
#include "topo_com.h"
#include "topo_api.h"
#include "smart_api.h"
#include "update_thread.h"
#include "smart_line_cpp_api.h"
#include "load_cfg.h"

#define capacity 10
static struct smart_cfg_list    g_cfg_list = {};
PriorityQueue                   g_H;

void entry_init(void)
{
	topo_start("rrtopo_conf.json");
	g_H = Initialize(capacity);
	update_start(1);
}

int main(int argc, char **argv)
{
	load_supex_args(&g_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	load_cfg_file(&g_cfg_list.file_info, g_cfg_list.argv_info.conf_name);

	g_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_cfg_list.func_info[APPLY_FUNC_ORDER].func = (SUPEX_TASK_CALLBACK)insert_call;
	g_cfg_list.entry_init = entry_init;

	smart_mount(&g_cfg_list);
	smart_start();
	Destroy(g_H);
	return 0;
}

