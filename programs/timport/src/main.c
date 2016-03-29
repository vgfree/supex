#include "pool_api.h"

#include "timport_cfg.h"
#include "timport_api.h"
#include "timport_task.h"

extern unsigned int g_max_req_size;

struct timport_cfg_list g_timport_cfg_list = {};

void timport_entry_init(void)
{
	g_max_req_size = 32 * 1024 * 1024;
	timport_task_init();
}

void timport_shut_down(void)
{
	timport_task_exit();
}

int main(int argc, char *argv[])
{
	load_supex_args(&g_timport_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	read_timport_cfg(&g_timport_cfg_list.file_info, g_timport_cfg_list.argv_info.conf_name);

	g_timport_cfg_list.entry_init = timport_entry_init;
	g_timport_cfg_list.shut_down = timport_shut_down;

	timport_mount(&g_timport_cfg_list);

	timport_start();

	return 0;
}

