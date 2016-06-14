#include <stdio.h>
#include "base/free_queue.h"

#include "core/evcs_module.h"
#include "core/evcs_events.h"
#include "core/evcs_kernel.h"
#include "spx_evcs.h"
#include "spx_evcs_module.h"

EVCS_MODULE_SETUP(kernel, kernel_init, kernel_exit, &g_kernel_evts);
EVCS_MODULE_SETUP(evcs, evcs_init, evcs_exit, &g_evcs_evts);

#include "_ev_coro.h"
#include "evcoro_async_tasks.h"

struct free_queue_list  glist;
char                    TASK[64] = { 0 };
char                    DATA[] = "GET / HTTP/1.0\r\nUser-Agent: curl/7.38.0\r\nHost: www.sina.com\r\nAccept: */*\r\n\r\n";

static bool task_lookup(void *user, void *task)
{
	return free_queue_pull(&glist, task);
}

static bool task_report(void *user, void *task)
{
	return free_queue_push(&glist, task);
}

void *task_handle(struct supex_evcoro *evcoro, int step)
{
	printf("111111111\n");

	struct supex_evcoro     *p_evcoro = supex_get_default();
	struct evcoro_scheduler *p_scheduler = p_evcoro->scheduler;
	struct xpool            *cpool = conn_xpool_find("www.sina.com", 80);

	struct async_evtasker   *tasker = evtask_initial(p_scheduler, 1, QUEUE_TYPE_FIFO, NEXUS_TYPE_SOLO);
	struct command_node     *command = evtask_command(tasker, PROTO_TYPE_HTTP, cpool, DATA, sizeof(DATA) - 1);

	evtask_install(tasker);

	evtask_startup(p_scheduler);

	printf("%s\n", command->cache.buff);

	evtask_distory(tasker);
}

void main(void)
{
	conn_xpool_init("www.sina.com", 80, 10, true);
	free_queue_init(&glist, sizeof(TASK), 8);

	task_report(NULL, TASK);
	/*load module*/
	EVCS_MODULE_MOUNT(kernel);
	EVCS_MODULE_ENTRY(kernel, true);

	struct evcs_argv_settings sets = {
		.num    = 5
		, .tsz  = sizeof(TASK)
		, .data = NULL
		, .task_lookup= task_lookup
		, .task_handle= task_handle
	};
	EVCS_MODULE_CARRY(evcs, &sets);
	EVCS_MODULE_MOUNT(evcs);
	EVCS_MODULE_ENTRY(evcs, true);

	/*work events*/
	EVCS_MODULE_START();
}

