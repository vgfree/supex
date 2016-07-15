#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>


#include "major/alive_api.h"
#include "alive_cpp_api.h"
#include "load_alive_cfg.h"

#include "minor/sniff_api.h"
#include "load_sniff_cfg.h"

#include "sniff_evcoro_lua_api.h"

struct alive_cfg_list   g_alive_cfg_list = {};
struct sniff_cfg_list   g_sniff_cfg_list = {};

bool sniff_task_report(void *user, void *task)
{
	bool ok = false;

	ok = tlpool_push(user, task, TLPOOL_TASK_SEIZE, 0);

	if (ok) {
		x_printf(D, "push queue ok!");
	} else {
		x_printf(E, "push queue fail!");
	}

	return ok;
}

bool sniff_task_lookup(void *user, void *task)
{
	bool ok = false;

	ok = tlpool_pull(user, task, TLPOOL_TASK_SEIZE, 0);

	if (ok) {
		x_printf(D, "pull queue ok!");
	}

	return ok;
}

static void alive_pthrd_init(void *user)
{
	ALIVE_WORKER_PTHREAD *p_alive_worker = user;

	p_alive_worker->mount = sniff_start(0);
}

/**
 * 根据条件编译初始化文件队列和内存队列
 */
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

	g_alive_cfg_list.func_info[ONLINE_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_alive_cfg_list.func_info[ONLINE_FUNC_ORDER].func = (TASK_VMS_FCB)alive_vms_online;

	g_alive_cfg_list.func_info[OFFLINE_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_alive_cfg_list.func_info[OFFLINE_FUNC_ORDER].func = (TASK_VMS_FCB)alive_vms_offline;

	g_alive_cfg_list.entry_init = alive_entry_init;

	g_alive_cfg_list.pthrd_init = alive_pthrd_init;

	alive_mount(&g_alive_cfg_list);

	// ---> init sniff

	snprintf(g_sniff_cfg_list.argv_info.conf_name,
		sizeof(g_sniff_cfg_list.argv_info.conf_name),
		"%s", g_alive_cfg_list.argv_info.conf_name);

	snprintf(g_sniff_cfg_list.argv_info.serv_name,
		sizeof(g_sniff_cfg_list.argv_info.serv_name),
		"%s", g_alive_cfg_list.argv_info.serv_name);

	snprintf(g_sniff_cfg_list.argv_info.msmq_name,
		sizeof(g_sniff_cfg_list.argv_info.msmq_name),
		"%s", g_alive_cfg_list.argv_info.msmq_name);

	load_sniff_cfg_file(&g_sniff_cfg_list.file_info, g_alive_cfg_list.argv_info.conf_name);

	g_sniff_cfg_list.task_lookup = sniff_task_lookup;
	g_sniff_cfg_list.task_report = sniff_task_report;

	g_sniff_cfg_list.vmsys_init = sniff_vms_init;

	sniff_mount(&g_sniff_cfg_list);

	alive_start();

	return 0;
}

