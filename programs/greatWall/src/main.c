#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "mq_api.h"

#include "swift_api.h"
#include "swift_cpp_api.h"
#include "load_swift_cfg.h"

#include "sniff_api.h"
#include "pool_api.h"
#include "load_sniff_cfg.h"
#include "apply_def.h"
#include "dams_cfg.h"
#include "switch_queue.h"

#include "sniff_evcoro_cpp_api.h"

#include "add_session_cmd.h"

struct swift_cfg_list   g_swift_cfg_list = {};
struct sniff_cfg_list   g_sniff_cfg_list = {};
struct dams_cfg_file    g_dams_cfg_file = {};

static void swift_pthrd_init(void *user)
{
	SWIFT_WORKER_PTHREAD *p_swift_worker = user;

	p_swift_worker->mount = sniff_start(p_swift_worker, p_swift_worker->index, g_dams_cfg_file.qtype);
}


void init_service_cnt(void)
{
	int i = 0;

	for (i = 0; i < g_dams_cfg_file.count && i < MAX_LINK_INDEX; i++) {
		char    *host = g_dams_cfg_file.links[i].host;
		int     port = g_dams_cfg_file.links[i].port;
		bool    ret = pool_api_init(host, port, g_dams_cfg_file.per_peak_cnt_count, false);

		if (!ret) {
			x_printf(E, "link %s:%d ERROR!", host, port);
		}
	}
}

/**
 * 根据条件编译初始化文件队列和内存队列
 */
static void swift_entry_init(void)
{

	app_queue_init();
	/*
	 * 初始化网络包统计
	 */
	//	if (!netpkg_init()) {
	//		exit(EXIT_FAILURE);
	//	}

	if (unlikely(!netpkg_init())) {
		exit(EXIT_FAILURE);
	}

	/*
	 * 初始化支持的命令
	 */
	init_session_cmd();
#ifdef OPEN_EVUV
	init_service_cnt();
#endif
}

static void swift_shut_down(void)
{
	const SWIFT_WORKER_PTHREAD      *swift_worker = g_swift_worker_pthread;
	const int                       swift_worker_total = SWIFT_WORKER_COUNTS;
	int                             i = 0;
	int                             thds = 0;

	struct ThreadSuspend *cond = NULL;

	New(cond);

	if (!ThreadSuspendInit(cond)) {
		exit(EXIT_FAILURE);
	}

	/*通过每个swift_worker挂起sniff_worker的所有线程*/
	for (i = 0; i < swift_worker_total; i++) {
		thds++;

		sniff_suspend_thread(swift_worker[i].mount, cond);
	}

	/*
	 * 等待所有sniff_worker挂起
	 */
	ThreadSuspendWait(cond, thds * SNIFF_WORKER_COUNTS);

	/*
	 * 由于 sniff_worker 线程还在挂起状态，所以不能释放挂起条件
	 */
}

/*
 * 每个swift_worker 对应一批（一个）sniff_worker
 * @see swift_pthrd_init()
 */
static void swift_reload_cfg(void)
{
	struct supex_argv       *argv = &g_swift_cfg_list.argv_info;
	struct swift_cfg_file   *fileinfo = &g_swift_cfg_list.file_info;

	const SWIFT_WORKER_PTHREAD      *swift_worker = g_swift_worker_pthread;
	const int                       swift_worker_total = SWIFT_WORKER_COUNTS;
	int                             i = 0;
	int                             thds = 0;
	bool                            ok = false;

	struct ThreadSuspend cond = NULLOBJ_THREADSUSPEND;

	/*通过每个swift_worker挂起sniff_worker的所有线程*/
	for (i = 0; i < swift_worker_total; i++) {
		thds++;

		sniff_suspend_thread(swift_worker[i].mount, &cond);
	}

	/*
	 * 等待所有sniff_worker挂起
	 */
	ThreadSuspendWait(&cond, thds * SNIFF_WORKER_COUNTS);

	/*
	 * 重新加载配置文件
	 */
	ok = reload_swift_cfg_file(fileinfo, argv->conf_name);

	if (ok) {
		char path[MAX_PATH_SIZE] = {};

		snprintf(path, sizeof(path), "%s/%s.log", fileinfo->log_path, fileinfo->log_file);

		SLogOpen(path, SLogIntegerToLevel(fileinfo->log_level));

		x_printf(D, "reload configure file [%s] success.", argv->conf_name);
	}

	ok = read_dams_cfg(&g_dams_cfg_file, argv->conf_name);

	if (ok) {
		x_printf(D, "reload configure file [%s] success.", argv->conf_name);
	}

#ifdef OPEN_EVUV
	init_service_cnt();
#endif

	/*
	 * 复位 sniff_worker
	 */
	ThreadSuspendEnd(&cond);
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

	g_swift_cfg_list.func_info[LPUSHX_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[LPUSHX_FUNC_ORDER].func = (TASK_CALLBACK)swift_vms_call_rlpushx;
	g_swift_cfg_list.func_info[RPUSHX_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[RPUSHX_FUNC_ORDER].func = (TASK_CALLBACK)swift_vms_call_rlpushx;
	g_swift_cfg_list.func_info[PUBLISH_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[PUBLISH_FUNC_ORDER].func = (TASK_CALLBACK)swift_vms_call_publish;

	g_swift_cfg_list.entry_init = swift_entry_init;

	g_swift_cfg_list.pthrd_init = swift_pthrd_init;

	g_swift_cfg_list.reload_cfg = swift_reload_cfg;

	g_swift_cfg_list.shut_down = swift_shut_down;

	/*have bug when tasks pile up if use idle task*/

	swift_mount(&g_swift_cfg_list);

	// ---> init route
	ok = read_dams_cfg(&g_dams_cfg_file, g_swift_cfg_list.argv_info.conf_name);

	if (!ok) {
		exit(EXIT_FAILURE);
	}

	// ---> init sniff

	snprintf(g_sniff_cfg_list.argv_info.conf_name,
		sizeof(g_sniff_cfg_list.argv_info.conf_name),
		"%s", g_swift_cfg_list.argv_info.conf_name);

	snprintf(g_sniff_cfg_list.argv_info.serv_name,
		sizeof(g_sniff_cfg_list.argv_info.serv_name),
		"%s", g_swift_cfg_list.argv_info.serv_name);

	snprintf(g_sniff_cfg_list.argv_info.msmq_name,
		sizeof(g_sniff_cfg_list.argv_info.msmq_name),
		"%s", g_swift_cfg_list.argv_info.msmq_name);

	load_sniff_cfg_file(&g_sniff_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	/*
	 *        g_sniff_cfg_list.func_info[ APPLY_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	 *        g_sniff_cfg_list.func_info[ APPLY_FUNC_ORDER ].func = (TASK_CALLBACK)sniff_vms_call;
	 */
	g_sniff_cfg_list.task_lookup = sniff_task_lookup;
	g_sniff_cfg_list.task_report = sniff_task_report;

	sniff_mount(&g_sniff_cfg_list);

	swift_start();

	return 0;
}

