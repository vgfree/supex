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

EVCS_MODULE_SETUP(kernel, kernel_init, kernel_exit, &g_kernel_evts);
EVCS_MODULE_SETUP(evcs, evcs_init, evcs_exit, &g_evcs_evts);

#include "_ev_coro.h"
#include "evcoro_async_tasks.h"
#include "thread_pool_loop/tlpool.h"

#define SET_DATA_PROCESS 0

#define MAX_SET_DATA_PROCESS_NUMBER 2

#define PROCESS_ID (MAX_SET_DATA_PROCESS_NUMBER + SET_DATA_PROCESS)

tlpool_t *tlpool = NULL;

struct free_queue_list  glist;
char TASK[64] = { 0 };

struct user_task {
	int TASK;
};

struct user_data {
	char *p_buf;
	struct redis_status *status;
};

struct user_data UserData; 

static bool task_lookup(void *user, void *task)
{
	tlpool_t *pool = user;
	bool ok = false;
	int idx = tlpool_get_thread_index(pool);

	ok = tlpool_pull(pool, task, TLPOOL_TASK_SEIZE, idx);
	if (ok) {
		return ok;
	}

	return ok;
}

static bool task_report(void *user, void *task)
{
	tlpool_t *pool = user;
	bool ok = false;
	ok = tlpool_push(pool, task, TLPOOL_TASK_SEIZE, NULL);
	return ok;
}

static void __ConfigSetProcess()
{
	int i = 0;
	struct user_task task;
	if (UserData.p_buf == NULL || UserData.status == NULL) {
		printf("Did not get data yet !\n");
		return;
	}
	
	if (!tlpool) {
		printf("The point of process pool is invalid.\n");
		exit(0);
	}

	i = UserData.status->fields + 1;

	while (i-- > 1) {
		printf("Start set data process, taskid = %d\n", i);
		task.TASK = i;
		//sprintf(task.TASK, "task %d", i);
		task_report(tlpool, &task);
	}
}

void GetData_task()
{
	int len = 0;
	char *proto = NULL;
	UserData.p_buf = NULL;
	UserData.status = NULL;

	struct supex_evcoro     *p_evcoro = supex_get_default();
	struct evcoro_scheduler *p_scheduler = p_evcoro->scheduler;
	struct xpool            *cpool = conn_xpool_find("192.168.1.12", 9001);
	len = cmd_to_proto(&proto, "SMEMBERS %s", "ACTIVEUSER:20160616104");
	struct async_evtasker   *tasker = evtask_initial(p_scheduler, 1, QUEUE_TYPE_FIFO, NEXUS_TYPE_SOLO);
	struct command_node     *command = evtask_command(tasker, PROTO_TYPE_REDIS, cpool, proto, len);

	evtask_install(tasker);
	evtask_startup(p_scheduler);

	printf("%.*s", command->cache.end - command->cache.start, &command->cache.buff[command->cache.start]);
	UserData.p_buf = cache_data_address(&command->cache);
        UserData.status = &command->parse.redis_info.rs;
	printf("redis data fields = %d\n", UserData.status->fields);

	__ConfigSetProcess();
	
	evtask_distory(tasker);
}

void *SetData_task_handle(struct supex_evcoro *evcoro, void *step)
{
	int len = 0;
	char *proto = NULL;
	char *time = "20160616104";

	int idx_task = (int)(uintptr_t)step;
	struct user_task *p_task = &((struct user_task *)evcoro->task)[idx_task];

	char *userName = malloc(UserData.status->field[p_task->TASK - 1].len + 1);
	memset(userName, 0, UserData.status->field[p_task->TASK - 1].len + 1);
	memcpy(userName, UserData.p_buf + UserData.status->field[p_task->TASK - 1].offset, UserData.status->field[p_task->TASK - 1].len);

	printf("Lawrence hamster said task_id = %d\n", p_task->TASK);
	printf("Lawrence hamster said data = %s\n", userName);

	dispatchToUser(userName, UserData.status->field[p_task->TASK - 1].len, time, strlen(time));

	free(userName);
}

void SetDataWork(void *data)
{
        /*load module*/
        EVCS_MODULE_MOUNT(kernel);
        EVCS_MODULE_ENTRY(kernel, true);

        struct evcs_argv_settings sets = {
                .num    = MAX_SET_DATA_PROCESS_NUMBER /*协程数*/
                , .tsz  = sizeof(struct user_task)
                , .data = data
                , .task_lookup= task_lookup
                , .task_handle= (void (*)(void *data,        void *addr))SetData_task_handle
        };
        EVCS_MODULE_CARRY(evcs, &sets);
        EVCS_MODULE_MOUNT(evcs);
        EVCS_MODULE_ENTRY(evcs, true);

        EVCS_MODULE_START();
}

static void exitFunction(tlpool_t *tlpool)
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

static int StartTime = 1466064900;

void TimeProcessFunction(startTime)
{
	printf("enter TimeProcessFunction\n");
	time_t t;
    	int currentTime;
	if (time(&t) > startTime) {
		printf("startTime is incorrent !\n");
		exit(0);
	}

	while ((currentTime = time(&t)) < startTime) {
		sleep(0.2);		
	}

	StartTime += 60;
	printf("exit TimeProcessFunction\n");
}

static bool task_lookup_main(void *user, void *task)
{
	return free_queue_pull(&glist, task);
}

static bool task_report_main(void *user, void *task)
{
	return free_queue_push(&glist, task);
}

void *getData_task_handle()
{
	while(1) {
		TimeProcessFunction(StartTime);
		GetData_task();
		sleep(1);
	}
}

int main(void)
{
	int processIndex;
	conn_xpool_init("192.168.1.12", 9001, 10, true);  //redis, for get data process
	tlpool = tlpool_init(PROCESS_ID, 100, sizeof(struct user_task), NULL);	

	for (processIndex = SET_DATA_PROCESS; processIndex < PROCESS_ID; processIndex++) {
		tlpool_bind(tlpool, (void (*)(void *))SetDataWork, tlpool, processIndex);
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
                , .task_handle= (void (*)(void *data,        void *addr))getData_task_handle
	};
        EVCS_MODULE_CARRY(evcs, &sets);
        EVCS_MODULE_MOUNT(evcs);
        EVCS_MODULE_ENTRY(evcs, true);

        EVCS_MODULE_START();

	exitFunction(tlpool);
	sleep(2);

	return 0;
}
