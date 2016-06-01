//
//  main.c
//  supex
//
//  Created by 周凯 on 15/10/22.
//  Copyright © 2015年 zk. All rights reserved.
//

#include <stdio.h>
#include "prg_frame.h"

int main(int argc, char **argv)
{
	/*
	 * 初始化支持的命令
	 */
	init_session_cmd();

	pool_api_init("127.0.0.1", 6000, 2000, true);

	pool_api_init("www.sina.com", 80, 2000, true);
}

#if 0
static void swift_shut_down(void)
{}

/*
 * 每个swift_worker 对应一批（一个）sniff_worker
 * @see swift_pthrd_init()
 */
static void swift_reload_cfg(void)
{
	struct supex_argv       *argv = &g_swift_cfg_list.argv_info;
	struct swift_cfg_file   *fileinfo = &g_swift_cfg_list.file_info;

	const SWIFT_WORKER_PTHREAD      *swift_worker = g_swift_worker_pthread;
	const int                       swift_worker_total = G_SWIFT_WORKER_COUNTS;
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
	ThreadSuspendWait(&cond, thds * G_SNIFF_WORKER_COUNTS);

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

	FINALLY
	{
		// clean
		finally();
	}
	END;

	return EXIT_SUCCESS;
}

