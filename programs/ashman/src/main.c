#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "mq_api.h"
// #include "luakvutils.h"

#include "swift_api.h"
#include "swift_cpp_api.h"
#include "load_swift_cfg.h"

#include "sniff_api.h"
#include "load_sniff_cfg.h"
#include "switch_queue.h"

#ifdef OPEN_SCCO
  #include "sniff_scco_lua_api.h"
#else
  #include "sniff_line_lua_api.h"
#endif

struct swift_cfg_list   g_swift_cfg_list = {};
struct sniff_cfg_list   g_sniff_cfg_list = {};

static void swift_pthrd_init(void *user)
{
	SWIFT_WORKER_PTHREAD *p_swift_worker = user;

	p_swift_worker->mount = sniff_start(p_swift_worker, p_swift_worker->index, 0);
}


void swift_entry_init(void)
{
	app_queue_init();

	// if (kv_init(NULL) != ERR_NONE) {
	//      fprintf(stdout, "libaray key-value initialize failed");
	//      exit(-1);
	// }
	if (!kvpool_init()) {
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	// ---> init swift
	load_supex_args(&g_swift_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	load_swift_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_CALLBACK)swift_vms_call;
	// g_swift_cfg_list.func_info[ FETCH_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	// g_swift_cfg_list.func_info[ FETCH_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_gain;
	// g_swift_cfg_list.func_info[ MERGE_FUNC_ORDER ].type = BIT8_TASK_TYPE_WHOLE;
	// g_swift_cfg_list.func_info[ MERGE_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_sync;
	g_swift_cfg_list.func_info[CUSTOM_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[CUSTOM_FUNC_ORDER].func = (TASK_CALLBACK)swift_vms_exec;

	g_swift_cfg_list.entry_init = swift_entry_init;
	g_swift_cfg_list.pthrd_init = swift_pthrd_init;

	g_swift_cfg_list.vmsys_init = swift_vms_init;
	// g_swift_cfg_list.vmsys_exit = swift_vms_exit;
	// g_swift_cfg_list.vmsys_cntl = swift_vms_cntl;
	// g_swift_cfg_list.vmsys_rfsh = swift_vms_rfsh;
#ifdef STORE_USE_UCMQ_AND_QUEUE
	// g_swift_cfg_list.vmsys_idle = swift_vms_idle;
	/*have bug when tasks pile up*/
#endif

	swift_mount(&g_swift_cfg_list);

	// ---> init sniff
	memcpy(g_sniff_cfg_list.argv_info.conf_name,
		g_swift_cfg_list.argv_info.conf_name, MAX_FILE_NAME_SIZE);
	memcpy(g_sniff_cfg_list.argv_info.serv_name,
		g_swift_cfg_list.argv_info.serv_name, MAX_FILE_NAME_SIZE);
	memcpy(g_sniff_cfg_list.argv_info.msmq_name,
		g_swift_cfg_list.argv_info.msmq_name, MAX_FILE_NAME_SIZE);

	load_sniff_cfg_file(&g_sniff_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	/*
	 *        g_sniff_cfg_list.func_info[ APPLY_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	 *        g_sniff_cfg_list.func_info[ APPLY_FUNC_ORDER ].func = (TASK_CALLBACK)sniff_vms_call;
	 */
	g_sniff_cfg_list.task_lookup = sniff_task_lookup;
	g_sniff_cfg_list.task_report = sniff_task_report;

	g_sniff_cfg_list.vmsys_init = sniff_vms_init;

	sniff_mount(&g_sniff_cfg_list);

	swift_start();
	return 0;
}

