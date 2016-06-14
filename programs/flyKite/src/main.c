#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include "mq_api.h"

#ifdef OPEN_TOPO
  #include "topo.h"
  #include "topo_api.h"
#endif

#include "major/swift_api.h"
#include "swift_cpp_api.h"
#include "load_swift_cfg.h"

#include "minor/sniff_api.h"
#include "load_sniff_cfg.h"
#include "base/switch_queue.h"
#include "app_queue.h"

#include "sniff_evcoro_lua_api.h"


struct swift_cfg_list   g_swift_cfg_list = {};
struct sniff_cfg_list   g_sniff_cfg_list = {};

static void swift_pthrd_init(void *user)
{
	SWIFT_WORKER_PTHREAD *p_swift_worker = user;

	p_swift_worker->mount = sniff_start(p_swift_worker->index);
}


static void swift_entry_init(void)
{
	app_queue_init();

#ifdef OPEN_TOPO
	topo_start("gopath_conf.json");
	// same_kill("gopath");
#endif

	//init_session_cmd();

	if (!kvpool_init()) {
		exit(EXIT_FAILURE);
	}
}

static void swift_shut_down()
{
	SWIFT_WORKER_PTHREAD    *swift_worker = g_swift_worker_pthread;
	const int               swift_worker_total = G_SWIFT_WORKER_COUNTS;
	int                     i = 0;
	int                     thds = 0;

	struct ThreadSuspend *cond = NULL;

	New(cond);

	if (!ThreadSuspendInit(cond)) {
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < swift_worker_total; i++) {
		thds++;
		sniff_suspend_thread(swift_worker[i].mount, cond);
	}

	/*
	 * 等待所有sniff_worker挂起
	 */
	ThreadSuspendWait(cond, thds * G_SNIFF_WORKER_COUNTS);

	/*
	 * 由于 sniff_worker 线程还在挂起状态，所以不能释放挂起条件
	 */
}

int main(int argc, char **argv)
{
	bool ok = false;

	// ---> init swift
	load_supex_args(&g_swift_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	ok = load_swift_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	if (!ok) {
		exit(EXIT_FAILURE);
	}

	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_call;
	// g_swift_cfg_list.func_info[ FETCH_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	// g_swift_cfg_list.func_info[ FETCH_FUNC_ORDER ].func = (TASK_VMS_FCB)swift_vms_gain;
	// g_swift_cfg_list.func_info[ MERGE_FUNC_ORDER ].type = BIT8_TASK_TYPE_WHOLE;
	// g_swift_cfg_list.func_info[ MERGE_FUNC_ORDER ].func = (TASK_VMS_FCB)swift_vms_sync;
	g_swift_cfg_list.func_info[CUSTOM_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[CUSTOM_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_exec;

	g_swift_cfg_list.entry_init = swift_entry_init;
	g_swift_cfg_list.pthrd_init = swift_pthrd_init;

	//g_swift_cfg_list.shut_down = swift_shut_down;

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

