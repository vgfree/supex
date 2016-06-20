#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include "core/evcs_module.h"
#include "core/evcs_events.h"
#include "core/evcs_kernel.h"
#include "spx_evcs.h"
#include "spx_evcs_module.h"
#include "async_tasks/async_obj.h"
#include "base/free_queue.h"
#include "timport_cfg.h"
#include "get_user_key.h"
#include "dispatch_data.h"

EVCS_MODULE_SETUP(kernel, kernel_init, kernel_exit, &g_kernel_evts);
EVCS_MODULE_SETUP(evcs, evcs_init, evcs_exit, &g_evcs_evts);

#include "_ev_coro.h"
#include "evcoro_async_tasks.h"
#include "thread_pool_loop/tlpool.h"

#define SET_DATA_PROCESS                0

#define MAX_SET_DATA_PROCESS_NUMBER     2

#define PROCESS_ID                      (MAX_SET_DATA_PROCESS_NUMBER + SET_DATA_PROCESS)

struct timport_cfg_list g_timport_cfg_list = {};

tlpool_t *tlpool = NULL;

struct free_queue_list  glist;
char TASK[64] = { 0 };

struct user_task
{
	int TASK;
	char *user;
	int redis_cnt;
};

struct user_data
{
	char                    *p_buf;
	struct redis_status     *status;
};
 
struct user_data  g_user_data;

extern struct user_key g_user_key;

static bool task_lookup(void *user, void *task)
{
	tlpool_t        *pool = user;
	bool            ok = false;
	int             idx = tlpool_get_thread_index(pool);

	ok = tlpool_pull(pool, task, TLPOOL_TASK_SEIZE, idx);

	if (ok) {
		return ok;
	}

	return ok;
}

static bool task_report(void *user, void *task)
{
	tlpool_t        *pool = user;
	bool            ok = false;

	ok = tlpool_push(pool, task, TLPOOL_TASK_SEIZE, NULL);
	return ok;
}

static void __set_task_config(int redis_cnt)
{
	int                     i = 0;
	struct user_task        task;
	task.redis_cnt = redis_cnt;	

	if ((g_user_data.p_buf == NULL) || (g_user_data.status == NULL)) {
		printf("Did not get data yet !\n");
		return;
	}

	if (!tlpool) {
		printf("The point of process pool is invalid.\n");
		exit(0);
	}

	i = g_user_data.status->fields + 1;

	while (i-- > 1) {
		printf("Start set data process, taskid = %d\n", i);
		task.TASK = i;
		task.user = malloc(g_user_data.status->field[i - 1].len + 1);
		memset(task.user, 0, g_user_data.status->field[i - 1].len + 1);
		memcpy(task.user, g_user_data.p_buf + g_user_data.status->field[i - 1].offset, g_user_data.status->field[i - 1].len);
		task_report(tlpool, &task);
	}
}

void get_data_task(int redis_cnt)
{
	int     len = 0;
	char    *proto = NULL;

	g_user_data.p_buf = NULL;
	g_user_data.status = NULL;
	struct supex_evcoro     *p_evcoro = supex_get_default();
	struct evcoro_scheduler *p_scheduler = p_evcoro->scheduler;
	struct xpool            *cpool = conn_xpool_find(g_timport_cfg_list.file_info.redis[redis_cnt].host, g_timport_cfg_list.file_info.redis[redis_cnt].port);

	get_user_key(g_timport_cfg_list.file_info.start_time, g_timport_cfg_list.file_info.time_interval);

	len = cmd_to_proto(&proto, "SMEMBERS %s", g_user_key.key);
	struct async_evtasker   *tasker = evtask_initial(p_scheduler, 1, QUEUE_TYPE_FIFO, NEXUS_TYPE_SOLO);
	struct command_node     *command = evtask_command(tasker, PROTO_TYPE_REDIS, cpool, proto, len);

	evtask_install(tasker);
	evtask_startup(p_scheduler);

	g_user_data.p_buf = cache_data_address(&command->cache);
	g_user_data.status = &command->parse.redis_info.rs;
	printf("redis data fields = %d\n", g_user_data.status->fields);
/*
	for (int j = 0; j < g_user_data.status->fields; j++) {
		printf("redis field[%d] = %s\n", j, g_user_data.p_buf + g_user_data.status->field[j].offset);
	}
*/
	__set_task_config(redis_cnt);

	evtask_distory(tasker);
}

void *set_data_task_handle(struct supex_evcoro *evcoro, void *step)
{
	int                     idx_task = (int)(uintptr_t)step;
	struct user_task        *p_task = &((struct user_task *)evcoro->task)[idx_task];

	//printf("taskid = %d, task->user = %s, length = %d\n", p_task->TASK, p_task->user, strlen(p_task->user));

	dispatch_data(p_task->user, strlen(p_task->user), g_user_key.key, strlen(g_user_key.key), p_task->redis_cnt);

	free(p_task->user);
}

void set_data_task(void *data)
{
	/*load module*/
	EVCS_MODULE_MOUNT(kernel);
	EVCS_MODULE_ENTRY(kernel, true);

	struct evcs_argv_settings sets = {
		.num    = MAX_SET_DATA_PROCESS_NUMBER	/*协程数*/
		, .tsz  = sizeof(struct user_task)
		, .data = data
		, .task_lookup= task_lookup
		, .task_handle= (void (*)(void *data,        void *addr))set_data_task_handle
	};
	EVCS_MODULE_CARRY(evcs, &sets);
	EVCS_MODULE_MOUNT(evcs);
	EVCS_MODULE_ENTRY(evcs, true);

	EVCS_MODULE_START();
}

static void __exit(tlpool_t *tlpool)
{
	sleep(3);
	printf("stop!\n");
	tlpool_stop(tlpool);

	sleep(6);
	printf("cont!\n");
	tlpool_cont(tlpool);

	sleep(7);
	printf("exit!\n");
	tlpool_exit(tlpool);

	sleep(1);
	tlpool_wait(tlpool);

	sleep(1);
	tlpool_free(tlpool);
}

static void __timer_start(int start_time)
{
	printf("enter timer_start\n");
	time_t  t;
	int     current_time;
/*
	if (time(&t) > startTime) {
		printf("startTime is incorrent !\n");
		exit(0);
	}
*/
	while ((current_time = time(&t)) < start_time) {
		sleep(0.2);
	}

	printf("exit timer_start\n");
}

static bool task_lookup_main(void *user, void *task)
{
	return free_queue_pull(&glist, task);
}

static bool task_report_main(void *user, void *task)
{
	return free_queue_push(&glist, task);
}

void *get_data_task_handle()
{
	int r_idx;
	while(1) {
		//定时函数start
		__timer_start(g_timport_cfg_list.file_info.start_time);
		for (r_idx = 0; r_idx < g_timport_cfg_list.file_info.redis_cnt; r_idx ++) {	
			get_data_task(r_idx);
			sleep(2);  // 2秒处理1个redis
		}
		
		//每次定时处理完之后，起始时间+时间间隔（分钟*60s）
		g_timport_cfg_list.file_info.start_time += 60 * g_timport_cfg_list.file_info.time_interval;
	}
}


int main(int argc, char *argv[])
{
	load_supex_args(&g_timport_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);
	read_timport_cfg(&g_timport_cfg_list.file_info, g_timport_cfg_list.argv_info.conf_name);

	int p_idx;

	int r_idx;
	for (r_idx = 0; r_idx < g_timport_cfg_list.file_info.redis_cnt; r_idx ++) {
		conn_xpool_init(g_timport_cfg_list.file_info.redis[r_idx].host, g_timport_cfg_list.file_info.redis[r_idx].port, 10, true);
	}

	tlpool = tlpool_init(PROCESS_ID, 100, sizeof(struct user_task), NULL);

	for (p_idx = SET_DATA_PROCESS; p_idx < PROCESS_ID; p_idx++) {
		tlpool_bind(tlpool, (void (*)(void *))set_data_task, tlpool, p_idx);
	}

	tlpool_boot(tlpool);

	free_queue_init(&glist, sizeof(TASK), 8);
	task_report_main(NULL, TASK);
	/*load module*/
	EVCS_MODULE_MOUNT(kernel);
	EVCS_MODULE_ENTRY(kernel, true);

	struct evcs_argv_settings sets = {
		.num    = 1
		, .tsz  = sizeof(TASK)
		, .data = NULL
		, .task_lookup= task_lookup_main
		, .task_handle= (void (*)(void *data, void *addr))get_data_task_handle
	};
	EVCS_MODULE_CARRY(evcs, &sets);
	EVCS_MODULE_MOUNT(evcs);
	EVCS_MODULE_ENTRY(evcs, true);

	EVCS_MODULE_START();

	__exit(tlpool);
	sleep(2);

	return 0;
}

