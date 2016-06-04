#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>

#include "worker.h"
#include "worker_mem.h"
#include "grp_pthread.h"
#include "log.h"
#include "utils.h"

#include "major/swift_api.h"
#include "major_def.h"
#include "load_swift_cfg.h"

int     g_update_data_hour;
int     g_update_data_min;
int     g_update_data_sec;
int     g_update_data_cycle = (24 * 60 * 60);

#define WORKER_TIMING_BEGIN     g_update_data_hour, g_update_data_min, g_update_data_sec/*hour,min,sec*/
#define WORKER_TIMING_CYCLE     g_update_data_cycle

struct swift_cfg_list g_swift_cfg_list = {};
// static struct safe_once_init do_once   = {};
static struct safe_once_init    do_once = { 0, NULLOBJ_AO_SPINLOCK };
extern int                      (*worker_timer_do)(void);

/*初始化，启动*/
void swift_entry_init(void)
{
	if (-1 == mem_init()) {
		log_info(LOG_E, "worker的内存初始化错误\n");
		exit(-1);
	}

	if (-1 == grp_thread_init()) {
		log_info(LOG_E, "接出线程群初始化失败\n");
		exit(-1);
	}

	if (-1 == grp_thread_boot()) {
		log_info(LOG_E, "接出线程群启动失败");
		exit(-1);
	}
}

/*剩余时间*/
static int left_time(int h, int m, int s)
{
	int             left = 0;
	time_t          tm;
	struct tm       *st = NULL;

	assert((h >= 0 && h <= 23) && (m >= 0 && m <= 59) && (s >= 0 && s <= 59));

	time(&tm);
	st = localtime(&tm);

	left = (h * 60 * 60 + m * 60 + s) - (st->tm_hour * 60 * 60 + st->tm_min * 60 + st->tm_sec);

	if (left < 0) {
		left += 24 * 60 * 60;
	}

	return left;
}

/*定时加载数据*/
static void time_to_load_data_cb(struct ev_loop *loop, ev_timer *w, int revents)
{
	if (NULL == worker_timer_do) {
		log_info(LOG_S, "没有为worker安装定时处理工作\n");
		ev_timer_stop(loop, w);
		return;
	}

	if (-1 == worker_timer_do()) {
		log_info(LOG_E, "定时加载内存失败\n");
	}

	log_info(LOG_D, "定时工作处理成功\n");
}

/*为每个worker初始化日志，并为注册定时加载任务*/
static void swift_pthrd_init(void *user)
{
	SWIFT_WORKER_PTHREAD    *worker = (SWIFT_WORKER_PTHREAD *)user;
	char                    logfile[512] = {};

	// snprintf(logfile,sizeof(logfile),"/data/supex/programs/releaseServer/log/%s","worker.log");
	snprintf(logfile, sizeof(logfile), "%s/log/%s", getenv("WORK_PATH"), "worker.log");
	log_init(logfile);

	SAFE_ONCE_INIT_COME(&do_once);
	worker->mount = (struct ev_timer *)calloc(1, sizeof(struct ev_timer));
	ev_timer_init((struct ev_timer *)worker->mount, time_to_load_data_cb, left_time(WORKER_TIMING_BEGIN), WORKER_TIMING_CYCLE);
	ev_timer_start(worker->loop, (struct ev_timer *)worker->mount);
	SAFE_ONCE_INIT_OVER(&do_once);
}

#define DATA_MAX_LEN (80 * 1024)
/*worker解析完数据后，调用此接口处理数据*/
int data_handle(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	SWIFT_WORKER_PTHREAD    *p_swift_worker = (SWIFT_WORKER_PTHREAD *)user;
	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *ptr = NULL;

	char    buf[DATA_MAX_LEN] = {};
	int     buflen = sizeof(buf);

	buflen = cache_get(&p_node->mdl_recv.cache, buf, buflen);		/*从worker那获取信息并读取数据*/

	/*=============调用客户化接口===============*/

	ptr = strstr(buf, "\r\n\r\n");

	if (!ptr) {
		log_info(LOG_E, "收到的http数据中没有\r\n\r\n");
		return -1;
	}

	buflen = buflen - (ptr + 4 - buf);

	if (-1 == worker_data_handle(ptr + 4, buflen)) {
		log_info(LOG_E, "worker_data_handle error\n");
		return -1;
	}

	/*===========调用客户化接口结束=============*/
	return 0;
}

int main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);

	load_supex_args(&g_swift_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	load_swift_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_VMS_FCB)data_handle;

	g_swift_cfg_list.entry_init = swift_entry_init;
	g_swift_cfg_list.pthrd_init = swift_pthrd_init;

	swift_mount(&g_swift_cfg_list);
	swift_start();
	return 0;
}

