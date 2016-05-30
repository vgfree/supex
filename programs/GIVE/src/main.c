#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include <openssl/evp.h>               
#include <openssl/aes.h>
#include <openssl/err.h>

#include "mq_api.h"

#include "swift_api.h"
#include "swift_cpp_api.h"
#include "load_swift_cfg.h"

#include "sniff_api.h"
#include "pool_api.h"
#include "load_sniff_cfg.h"
#include "apply_def.h"
#include "rr_cfg.h"
#include "switch_queue.h"

#include "sniff_evuv_cpp_api.h"

#include "add_session_cmd.h"

#include "redis_kv.h"
#include "libkv.h"
#include "loadfile_tomem.h"
#include "statistics_thread.h"

struct swift_cfg_list   g_swift_cfg_list = {};
struct sniff_cfg_list   g_sniff_cfg_list = {};
struct rr_cfg_file      g_rr_cfg_file = {};

kv_handler_t    *city_handler;
kv_handler_t    *count_handler;

static int loadGridkv(const char *file)
{
	city_handler = kv_create(NULL);
	FILE *fp = load_file(file);

	if (file_tokv(fp, city_handler)) {
		x_printf(E, "load Gridkv failed.! \n");
		return -1;
	}

	return 0;
}

static void swift_pthrd_init(void *user)
{
	SWIFT_WORKER_PTHREAD *p_swift_worker = user;

	p_swift_worker->mount = sniff_start(p_swift_worker, p_swift_worker->index, 0);
}


/**
 * 根据条件编译初始化文件队列和内存队列
 */
static void swift_entry_init(void)
{

	app_queue_init();
	/*
	 * 初始化支持的命令
	 */
	init_session_cmd();

	pool_api_init(g_rr_cfg_file.gaode_host, g_rr_cfg_file.gaode_port, 200, true);

	// init grid file
	loadGridkv("Hkeys-ct");

	// init count handler libev
	count_handler = kv_create(NULL);

	// init statistics pthread
	statistics_start(1);

        //openssl init
        OpenSSL_add_all_algorithms();
        ERR_load_crypto_strings();
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
		struct mount_info *link = NULL;

		int j = 0;

		link = (struct mount_info *)swift_worker[i].mount;

		for (j = 0; j < LIMIT_CHANNEL_KIND; j++) {
			thds++;

			sniff_suspend_thread(link[j].list, cond);
		}
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
		struct mount_info *link = NULL;

		int j = 0;

		link = (struct mount_info *)swift_worker[i].mount;

		for (j = 0; j < LIMIT_CHANNEL_KIND; j++) {
			thds++;

			sniff_suspend_thread(link[j].list, &cond);
		}
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

	ok = read_rr_cfg(&g_rr_cfg_file, argv->conf_name);

	if (ok) {
		x_printf(D, "reload configure file [%s] success.", argv->conf_name);
	}

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

	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_CALLBACK)swift_vms_call;

	g_swift_cfg_list.entry_init = swift_entry_init;

	g_swift_cfg_list.pthrd_init = swift_pthrd_init;

	g_swift_cfg_list.vmsys_init = swift_vms_init;

	g_swift_cfg_list.reload_cfg = swift_reload_cfg;

	g_swift_cfg_list.shut_down = swift_shut_down;

	swift_mount(&g_swift_cfg_list);

	// ---> init route
	ok = read_rr_cfg(&g_rr_cfg_file, g_swift_cfg_list.argv_info.conf_name);

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

	g_sniff_cfg_list.task_lookup = sniff_task_lookup;
	g_sniff_cfg_list.task_report = sniff_task_report;

	g_sniff_cfg_list.vmsys_init = sniff_vms_init;

	sniff_mount(&g_sniff_cfg_list);

	swift_start();

	kvhandler_destroy(count_handler);
	kvhandler_destroy(city_handler);
        ERR_free_strings();
        EVP_cleanup();
	return 0;
}

