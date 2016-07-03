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
#include "set_expire_time.h"
#include "get_start_time.h"

EVCS_MODULE_SETUP(kernel, kernel_init, kernel_exit, &g_kernel_evts);
EVCS_MODULE_SETUP(evcs, evcs_init, evcs_exit, &g_evcs_evts);

#include "_ev_coro.h"
#include "evcoro_async_tasks.h"
#include "thread_pool_loop/tlpool.h"

#define MAX_UTHREAD_COUNT       128
#define MAX_PTHREAD_COUNT       16

#define MAX_USER_KEY_LENGTH     64

struct timport_cfg_list g_timport_cfg_list = {};

struct free_queue_list  g_main_list;
char                    MAIN_TASK[64] = { 0 };

char g_user_key[MAX_USER_KEY_LENGTH];

tlpool_t *tlpool = NULL;

static g_start_time = 0;

struct user_task
{
	char    *user;
	int     redis_cnt;
};

struct user_data
{
	char                    *p_buf;
	struct redis_status     *status;
};

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

	ok = tlpool_push(pool, task, TLPOOL_TASK_SEIZE, 0);
	return ok;
}

static void __set_task_config(char *buf, struct redis_status *status, int redis_cnt)
{
	struct user_task task;

	task.redis_cnt = redis_cnt;

	if ((buf == NULL) || (status == NULL)) {
		printf("Did not get data yet !\n");
		return;
	}

	if (!tlpool) {
		printf("The point of process pool is invalid.\n");
		exit(0);
	}

	int idx;

	for (idx = 0; idx < status->fields; idx++) {
		task.user = malloc(status->field[idx].len + 1);
		memset(task.user, 0, status->field[idx].len + 1);
		memcpy(task.user, buf + status->field[idx].offset, status->field[idx].len);
		task_report(tlpool, &task);
	}
}

int get_data_task(int redis_cnt)
{
	char                    *p_buf = NULL;
	struct redis_status     *status = NULL;

	struct supex_evcoro     *p_evcoro = supex_get_default();
	struct evcoro_scheduler *p_scheduler = p_evcoro->scheduler;
	struct xpool            *cpool = conn_xpool_find(g_timport_cfg_list.file_info.redis[redis_cnt].host, g_timport_cfg_list.file_info.redis[redis_cnt].port);

	memset(g_user_key, 0, MAX_USER_KEY_LENGTH);
	get_user_key(g_start_time, g_user_key, MAX_USER_KEY_LENGTH);
	printf("g_user_key = %s, len = %d\n", g_user_key, strlen(g_user_key));
	char                    *proto = NULL;
	int                     len = cmd_to_proto(&proto, "SMEMBERS %s", g_user_key);
	struct async_evtasker   *tasker = evtask_initial(p_scheduler, 1, QUEUE_TYPE_FIFO, NEXUS_TYPE_SOLO);
	struct command_node     *command = evtask_command(tasker, PROTO_TYPE_REDIS, cpool, proto, len);

	evtask_install(tasker);
	evtask_startup(p_scheduler);

	p_buf = cache_data_address(&command->cache);
	status = &command->parse.redis_info.rs;
	printf("redis data fields = %d\n", status->fields);
		
	int redis_fields = status->fields;	

	__set_task_config(p_buf, status, redis_cnt);

	evtask_distory(tasker);
	
	return redis_fields;
}

void *set_data_task_handle(struct supex_evcoro *evcoro, int step)
{
	lua_State *L = NULL;
	struct user_task        *p_task = &((struct user_task *)evcoro->task)[step];
	union virtual_system    *p_VMS = &((union virtual_system *)evcoro->VMS)[step];
	if (p_VMS) {
		L = p_VMS->L;
	}
	else {
		L = lua_vm_init();
		evcoro->VMS[step].L = L;
	}
	
	if (!L) {
		printf("lua vm is NULL\n");
		free(p_task->user);
		exit(0);
	}
	
	dispatch_data(L, p_task->user, strlen(p_task->user), g_user_key, strlen(g_user_key), p_task->redis_cnt);
	free(p_task->user);
}

void task_worker(void *data)
{
	/*load module*/
	EVCS_MODULE_MOUNT(kernel);
	EVCS_MODULE_ENTRY(kernel, true);

	struct evcs_argv_settings sets = {
		.num    = MAX_UTHREAD_COUNT	/*协程数*/
		, .tsz  = sizeof(struct user_task)
		, .data = data
		, .task_lookup= task_lookup
		, .task_handle= set_data_task_handle
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
	/*
	 *        if (time(&t) > start_time) {
	 *                printf("startTime is incorrent !\n");
	 *                exit(0);
	 *        }
	 */
	while ((time(&t) - g_timport_cfg_list.file_info.delay_time) < start_time) {
		sleep(5);
	}

	printf("exit timer_start\n");
}

static bool task_lookup_main(void *user, void *task)
{
	return free_queue_pull(&g_main_list, task);
}

static bool task_report_main(void *user, void *task)
{
	return free_queue_push(&g_main_list, task);
}

void *get_data_task_handle(struct supex_evcoro *evcoro, void *step)
{
	int r_idx;
	int redis_fields = 0;
	while(1) {
		//定时函数start
		__timer_start(g_start_time);
		for (r_idx = 0; r_idx < g_timport_cfg_list.file_info.redis_cnt; r_idx ++) {	
			redis_fields = get_data_task(r_idx);
			// set expire time,  need g_user_key and r_idx.
			if (redis_fields > 0) {
				set_expire_time(g_user_key, strlen(g_user_key), r_idx);
			}
			sleep(2);  // 2秒处理1个redis
		}

		// 每次定时处理完之后，起始时间+时间间隔（分钟*60s）
		g_start_time = get_start_time(g_start_time);
		write_timestamp(g_start_time);
	}
}

int main(int argc, char *argv[])
{
	g_start_time = read_timestamp();
	load_supex_args(&g_timport_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);
	read_timport_cfg(&g_timport_cfg_list.file_info, g_timport_cfg_list.argv_info.conf_name);

	/*start thread pool*/
	int idx;

	for (idx = 0; idx < g_timport_cfg_list.file_info.redis_cnt; idx++) {
		conn_xpool_init(g_timport_cfg_list.file_info.redis[idx].host, g_timport_cfg_list.file_info.redis[idx].port, 10, true);
	}

	tlpool = tlpool_init(MAX_PTHREAD_COUNT, 100, sizeof(struct user_task), NULL);

	for (idx = 0; idx < MAX_PTHREAD_COUNT; idx++) {
		tlpool_bind(tlpool, (void (*)(void *))task_worker, tlpool, idx);
	}

	tlpool_boot(tlpool);

	/*start main loop*/
	free_queue_init(&g_main_list, sizeof(MAIN_TASK), 8);
	task_report_main(NULL, MAIN_TASK);
	/*load module*/
	EVCS_MODULE_MOUNT(kernel);
	EVCS_MODULE_ENTRY(kernel, true);

	struct evcs_argv_settings sets = {
		.num    = 1
		, .tsz  = sizeof(MAIN_TASK)
		, .data = NULL
		, .task_lookup= task_lookup_main
		, .task_handle= get_data_task_handle
	};
	EVCS_MODULE_CARRY(evcs, &sets);
	EVCS_MODULE_MOUNT(evcs);
	EVCS_MODULE_ENTRY(evcs, true);

	EVCS_MODULE_START();

	__exit(tlpool);
	sleep(2);

	return 0;
}

