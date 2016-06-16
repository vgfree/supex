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

struct user_task
{
	char TASK[64];
};

char DATA[] = "GET / HTTP/1.0\r\nUser-Agent: curl/7.38.0\r\nHost: www.sina.com\r\nAccept: */*\r\n\r\n";

static bool task_lookup(void *user, void *task)
{
	tlpool_t        *pool = user;
	bool            ok = false;
	int             idx = tlpool_get_thread_index(pool);

	ok = tlpool_pull(pool, task, TLPOOL_TASK_SHARE, idx);

	if (ok) {
		printf("get from share queue!\n");
		return ok;
	}

	ok = tlpool_pull(pool, task, TLPOOL_TASK_SEIZE, idx);

	if (ok) {
		printf("get from seize queue!\n");
		return ok;
	}

	ok = tlpool_pull(pool, task, TLPOOL_TASK_ALONE, idx);

	if (ok) {
		printf("get from alone queue!\n");
		return ok;
	}

	return ok;
}

static bool task_report(void *user, void *task)
{
	tlpool_t        *pool = user;
	bool            ok = false;

	ok = tlpool_push(pool, task, TLPOOL_TASK_SEIZE, 0);
	ok = tlpool_push(pool, task, TLPOOL_TASK_SHARE, 1);
	ok = tlpool_push(pool, task, TLPOOL_TASK_ALONE, 2);
	return ok;
}

void *task_handle(struct supex_evcoro *evcoro, int step)
{
	struct user_task        *p_task = &((struct user_task *)evcoro->task)[step];
	int                     idx = tlpool_get_thread_index(evcoro->data);

	printf("thread %p index %d get [%s]\n", pthread_self(), idx, p_task->TASK);
	printf("------------华丽的分割线------------\n");
#if 0
	struct supex_evcoro     *p_evcoro = supex_get_default();
	struct evcoro_scheduler *p_scheduler = p_evcoro->scheduler;
	struct xpool            *cpool = conn_xpool_find("www.sina.com", 80);

	struct async_evtasker   *tasker = evtask_initial(p_scheduler, 1, QUEUE_TYPE_FIFO, NEXUS_TYPE_SOLO);
	struct command_node     *command = evtask_command(tasker, PROTO_TYPE_HTTP, cpool, DATA, sizeof(DATA) - 1);

	evtask_install(tasker);

	evtask_startup(p_scheduler);

	printf("%s\n", command->cache.buff);

	evtask_distory(tasker);
#endif
}

void work(void *data)
{
	tlpool_t *pool = data;

	/*load module*/
	EVCS_MODULE_MOUNT(kernel);
	EVCS_MODULE_ENTRY(kernel, true);

	struct evcs_argv_settings sets = {
		.num    = 5	/*协程数*/
		, .tsz  = sizeof(struct user_task)
		, .data = data
		, .task_lookup= task_lookup
		, .task_handle= (void (*)(void *data,        void *addr))task_handle
	};
	EVCS_MODULE_CARRY(evcs, &sets);
	EVCS_MODULE_MOUNT(evcs);
	EVCS_MODULE_ENTRY(evcs, true);

	/*work events*/
	EVCS_MODULE_START();
}

void main(void)
{
	conn_xpool_init("www.sina.com", 80, 10, true);

	tlpool_t *tlpool = tlpool_init(3, 100, sizeof(struct user_task), NULL);

	tlpool_bind(tlpool, (void (*)(void *))work, tlpool, 0);
	tlpool_bind(tlpool, (void (*)(void *))work, tlpool, 1);
	tlpool_bind(tlpool, (void (*)(void *))work, tlpool, 2);

	tlpool_boot(tlpool);

	int i = 1;

	while (i-- > 0) {
		struct user_task task = { 0 };
		sprintf(task.TASK, "task %d", i);
		task_report(tlpool, &task);
		sleep(5);
	}

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

