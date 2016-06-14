#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include "core/evcs_module.h"
#include "core/evcs_events.h"
#include "core/evcs_kernel.h"
#include "spx_evcs.h"
#include "spx_evcs_module.h"

EVCS_MODULE_SETUP(kernel, kernel_init, kernel_exit, &g_kernel_evts);
EVCS_MODULE_SETUP(evcs, evcs_init, evcs_exit, &g_evcs_evts);

#include "_ev_coro.h"
#include "evcoro_async_tasks.h"
#include "thread_pool_loop/tlpool.h"

#define GET_DATA_PROCESS 0
#define SET_DATA_PROCESS 1

#define MAX_SET_DATA_PROCESS_NUMBER 2

#define PROCESS_ID (MAX_SET_DATA_PROCESS_NUMBER + SET_DATA_PROCESS)

//static bool IsGotData = false;

tlpool_t *tlpool = NULL;

struct user_task {
	char TASK[64];
};

static bool task_lookup(void *user, void *task)
{
	tlpool_t *pool = user;
	bool ok = false;
	int idx = tlpool_get_thread_index(pool);

	ok = tlpool_pull(pool, task, TLPOOL_TASK_ALONE, idx);
	if (ok) {
		printf("get from share queue!\n");
		return ok;
	}

	return ok;
}

static bool task_report(void *user, void *task, int taskId)
{
	tlpool_t *pool = user;
	bool ok = false;
	ok = tlpool_push(pool, task, TLPOOL_TASK_ALONE, taskId);
	return ok;
}

void *GetData_task_handle(struct supex_evcoro *evcoro, void *step)
{
	int i;
	int len = 0;
	char *proto = NULL;
	//int idx_task = (int)(uintptr_t)step;
	//struct user_task *p_task = &((struct user_task *)evcoro->task)[idx_task];
	//int idx = tlpool_get_thread_index(evcoro->data);
#if 1
	struct supex_evcoro     *p_evcoro = supex_get_default();
	struct evcoro_scheduler *p_scheduler = p_evcoro->scheduler;
	struct xpool            *cpool = conn_xpool_find("192.168.1.12", 9001);
	len = cmd_to_proto(&proto, "SMEMBERS %s", "ACTIVEUSER:20160612171");
	struct async_evtasker   *tasker = evtask_initial(p_scheduler, 1, QUEUE_TYPE_FIFO, NEXUS_TYPE_SOLO);
	struct command_node     *command = evtask_command(tasker, PROTO_TYPE_REDIS, cpool, proto, len);

	evtask_install(tasker);

	evtask_startup(p_scheduler);

	printf("%s\n", command->cache.buff);

	//Start set data process
	i = PROCESS_ID;
	while (i-- > 1) {
		printf("Start set data process, PROCESS_ID = %d\n", i);
		struct user_task task = {0};
		sprintf(task.TASK, "task %d", i);
		task_report(tlpool, &task, i);
	}

	evtask_distory(tasker);
#endif
}

void *SetData_task_handle(struct supex_evcoro *evcoro, void *step)
{
	int len = 0;
	char *proto = NULL;
	//int idx_task = (int)(uintptr_t)step;
	//struct user_task *p_task = &((struct user_task *)evcoro->task)[idx_task];
	//int idx = tlpool_get_thread_index(evcoro->data);
#if 1
	struct supex_evcoro     *p_evcoro = supex_get_default();
	struct evcoro_scheduler *p_scheduler = p_evcoro->scheduler;
	struct xpool            *cpool = conn_xpool_find("192.168.1.12", 9001);
	len = cmd_to_proto(&proto, "GET %s", "11");
	struct async_evtasker   *tasker = evtask_initial(p_scheduler, 1, QUEUE_TYPE_FIFO, NEXUS_TYPE_SOLO);
	struct command_node     *command = evtask_command(tasker, PROTO_TYPE_REDIS, cpool, proto, len);

	evtask_install(tasker);

	evtask_startup(p_scheduler);

	printf("%s\n", command->cache.buff);

	evtask_distory(tasker);
#endif
}

void GetDataWork(void *data)
{
	/*load module*/
	EVCS_MODULE_MOUNT(kernel);
	EVCS_MODULE_ENTRY(kernel, true);

	struct evcs_argv_settings sets = {
		.num    = 1/*协程数*/
		, .tsz  = sizeof(struct user_task)
		, .data = data
		, .task_lookup= task_lookup
		, .task_handle= (void (*)(void *data,        void *addr))GetData_task_handle
	};
	EVCS_MODULE_CARRY(evcs, &sets);
	EVCS_MODULE_MOUNT(evcs);
	EVCS_MODULE_ENTRY(evcs, true);

	/*work events*/
	EVCS_MODULE_START();
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

static int StartTime = 1465906140;

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

int main(void)
{
	int processIndex;
	conn_xpool_init("192.168.1.12", 9001, 10, true);  //redis, for get data process
	
	tlpool = tlpool_init(PROCESS_ID, 100, sizeof(struct user_task));	

	printf("before tlpool_bind !\n");

	tlpool_bind(tlpool, (void (*)(void *))GetDataWork, tlpool, GET_DATA_PROCESS);	

	for (processIndex = SET_DATA_PROCESS; processIndex < PROCESS_ID; processIndex++) {
		tlpool_bind(tlpool, (void (*)(void *))SetDataWork, tlpool, processIndex);
	}	

	tlpool_boot(tlpool);
	//conn_xpool_init(host, port, 10, true);		  //tsdb,  for set data process
	while(1) {
		TimeProcessFunction(StartTime);

		printf("after tlpool_boot\n");

		struct user_task task = {0};
		sprintf(task.TASK, "task %d", GET_DATA_PROCESS);
		task_report(tlpool, &task, GET_DATA_PROCESS);
		sleep(1);
	}

	exitFunction(tlpool);
	sleep(2);

	return 0;
}
