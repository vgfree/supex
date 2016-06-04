#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "major/swift_api.h"
#include "swift_lua_api.h"
#include "load_swift_cfg.h"
#include "load_child_cfg.h"
#include "share_evcb.h"
#include "list_node.h"
#include "pools/xpool.h"
#include "minor/crzpt_api.h"
#include "base/utils.h"
#include "share_evcb.h"

// 时间链表头节点
struct t_node *g_t_head = NULL;

struct ev_loop *g_child_loop = NULL;

short g_child_index = -1;

struct swift_cfg_list   g_swift_cfg_list = {};
struct child_cfg_list   g_child_cfg_list = {};

void msmq_share_cb(struct ev_loop *loop, ev_io *w, int revents)
{
	        int n = msmq_call();

		        x_printf(D, "done shell cntl %d", n);
}

static void child_init(void)
{
	char path[MAX_PATH_SIZE] = {};

	//	struct swift_cfg_file *p_cfg_file = &(g_swift_cfg_list.file_info);
	// printf("child init, host:%s, port:%d\n", g_child_cfg_list.app_redis_host[g_child_index], g_child_cfg_list.app_redis_port[g_child_index]);

	conn_xpool_init(g_child_cfg_list.app_redis_host[g_child_index], g_child_cfg_list.app_redis_port[g_child_index], 200000, true);

	//	snprintf(path, sizeof(path), "%s/%s.log", p_cfg_file->log_path, p_cfg_file->log_file);

	//	SLogOpen(path, SLogIntegerToLevel(p_cfg_file->log_level));
}

static void entry_init(void)
{
	if (!kvpool_init()) {
		exit(EXIT_FAILURE);
	}
}

static void accept_cntl(const char *data)
{
	struct msg_info *msg = (struct msg_info *)data;

	// x_printf(D, "mode is %c", msg->mode);
	switch (msg->mode)
	{
		case CRZPT_MSMQ_TYPE_PUSH:
		case CRZPT_MSMQ_TYPE_PULL:
			break;

		case CRZPT_MSMQ_TYPE_INSERT:

			// x_printf(I, "%s", msg->data);
			if (g_t_head == NULL) {
				time_node_init(&g_t_head);
			}

			// 把接收到的数据处理后写入redis
			data_to_redis(g_t_head, msg->data, g_child_loop,
				g_child_cfg_list.app_redis_host[g_child_index],
				g_child_cfg_list.app_redis_port[g_child_index]);
			break;

		default:
			// x_printf(E, "UNKNOW CMD [%c]!", msg->mode);
			return;
	}
}

int main(int argc, char **argv)
{
	int     i = 0;
	pid_t   pid[MAX_APP_COUNTS] = { 0 };

	SetProgName(argv[0]);

	/*load swift config*/
	load_supex_args(&g_swift_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);
	load_swift_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);
	/*load child config*/
	load_child_cfg_file(&g_child_cfg_list, g_swift_cfg_list.argv_info.conf_name);

	for (i = 0; i < g_child_cfg_list.app_counts; i++) {
		// memcpy( g_child_cfg_list[i].argv_info.conf_name,
		//		g_swift_cfg_list.argv_info.conf_name, MAX_FILE_NAME_SIZE );
		// memcpy( g_child_cfg_list[i].argv_info.serv_name,
		//		g_child_cfg_list.app_names[i], MAX_FILE_NAME_SIZE );
		// memcpy( g_child_cfg_list[i].argv_info.msmq_name,
		//		g_child_cfg_list.app_msmqs[i], MAX_FILE_NAME_SIZE );

		// g_child_cfg_list[i].file_info.log_path = x_strdup(g_swift_cfg_list.file_info.log_path);
		// g_child_cfg_list[i].file_info.log_file = x_strdup(g_swift_cfg_list.file_info.log_file);
		// g_child_cfg_list[i].file_info.log_level = g_swift_cfg_list.file_info.log_level;
	}

	/*config function*/
	for (i = 0; i < g_child_cfg_list.app_counts; i++) {
		ForkFrontEndStart(&pid[i]);

		g_child_index = i;
		child_init();
		/*====child====*/
		g_child_loop = ev_loop_new(EVBACKEND_EPOLL | EVFLAG_NOENV);
		struct ev_io msmq_watcher;		/* msmq watcher for new message */

		/*set msmq*/
		msmq_init(g_child_cfg_list.app_msmqs[i], accept_cntl);

		ev_io_init(&msmq_watcher, msmq_share_cb, msmq_hand(), EV_READ);
		ev_io_start(g_child_loop, &msmq_watcher);

		// x_printf(D, "start ...");
		ev_loop(g_child_loop, 0);
		ev_loop_destroy(g_child_loop);

		msmq_exit();

		ForkFrontEndTerm();
		// printf("FORK ONE PROCESS -->PID :%d\n", pid[i]);
	}

	/*====swift====*/
	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_call;
	g_swift_cfg_list.func_info[CUSTOM_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[CUSTOM_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_exec;

	g_swift_cfg_list.entry_init = entry_init;

	g_swift_cfg_list.vmsys_init = swift_vms_init;
	g_swift_cfg_list.vmsys_exit = swift_vms_exit;
	g_swift_cfg_list.vmsys_cntl = swift_vms_cntl;
	g_swift_cfg_list.vmsys_rfsh = swift_vms_rfsh;

	swift_mount(&g_swift_cfg_list);
	swift_start();
	return 0;
}

