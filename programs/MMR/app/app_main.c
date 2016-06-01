#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "mq_api.h"
#include "load_smart_cfg.h"
#include "entry.h"

#ifdef OPEN_TOPO
  #include "topo.h"
  #include "topo_api.h"
#endif

#include "swift_api.h"
#include "swift_cpp_api.h"
#include "load_swift_cfg.h"

#include "sniff_api.h"
#include "load_sniff_cfg.h"
#include "switch_queue.h"

#ifdef OPEN_SCCO
  #include "smart_scco_cpp_api.h"
  #include "sniff_scco_lua_api.h"
#else
  #include "smart_line_cpp_api.h"
  #include "sniff_line_lua_api.h"
#endif

#include "add_session_cmd.h"

struct smart_cfg_list   g_smart_cfg_list = {};
struct swift_cfg_list   g_swift_cfg_list = {};
struct sniff_cfg_list   g_sniff_cfg_list = {};

static void swift_pthrd_init(void *user)
{
	SWIFT_WORKER_PTHREAD *p_swift_worker = user;

	p_swift_worker->mount = sniff_start(p_swift_worker, p_swift_worker->index, 0);
}

#ifdef STORE_USE_QUEUE
static bool sniff_task_report(void *user, void *task)
{
	bool ok = false;

	ok = free_queue_push(&((SNIFF_WORKER_PTHREAD *)user)->tlist, task);

	if (ok) {
		x_printf(D, "push queue ok!");
		AO_INC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
	} else {
		x_printf(D, "push queue fail!");
	}

	return ok;
}

static bool sniff_task_lookup(void *user, void *task)
{
	bool ok = false;

	ok = free_queue_pull(&((SNIFF_WORKER_PTHREAD *)user)->tlist, task);

	if (ok) {
		x_printf(D, "pull queue ok!");
		AO_DEC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
	}

	return ok;
}
#endif	/* ifdef STORE_USE_QUEUE */

#if defined(STORE_USE_SHMQ)

extern ShmQueueT g_tasks_shmqueue;

static bool sniff_task_report(void *user, void *task)
{
	bool ok = false;

	//        ok = free_queue_push(&((SNIFF_WORKER_PTHREAD *)user)->tlist, task);

	ok = SHM_QueuePush(g_tasks_shmqueue, task, sizeof(struct sniff_task_node), NULL);

	if (ok) {
		x_printf(D, "push queue ok!");
		AO_INC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
	} else {
		x_printf(D, "push queue fail!");
	}

	return ok;
}

static bool sniff_task_lookup(void *user, void *task)
{
	bool ok = false;

	//        ok = free_queue_pull(&((SNIFF_WORKER_PTHREAD *)user)->tlist, task);

	ok = SHM_QueuePull(g_tasks_shmqueue, task, sizeof(struct sniff_task_node), NULL);

	if (ok) {
		x_printf(D, "pull queue ok!");
		AO_DEC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
	}

	return ok;
}
#endif	/* if defined(STORE_USE_SHMQ) */

#ifdef STORE_USE_UCMQ
static bool sniff_task_report(void *user, void *task)
{
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	char                    temp[32] = {};

	sprintf(temp, "%d_%d", p_sniff_worker->batch, p_sniff_worker->index);
	bool ok = mq_store_put(temp, task, sizeof(struct sniff_task_node));

	if (ok) {
		AO_INC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
		x_printf(D, "push queue ok!");
	} else {
		x_printf(D, "push queue fail!");
	}

	return ok;
}

static bool sniff_task_lookup(void *user, void *task)
{
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	char                    temp[32] = {};

	sprintf(temp, "%d_%d", p_sniff_worker->batch, p_sniff_worker->index);
	bool ok = mq_store_get(temp, task, sizeof(struct sniff_task_node));

	if (ok) {
		x_printf(D, "pull queue ok!");
		AO_DEC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
	}

	return ok;
}
#endif	/* ifdef STORE_USE_UCMQ */

#ifdef STORE_USE_UCMQ_AND_QUEUE
static struct switch_queue_info *g_queue_stat_list = NULL;

/*push*/
static bool major_push_call(struct switch_queue_info *p_stat, struct supex_task_node *p_node, va_list *ap)
{
	void                    *user = va_arg(*ap, void *);
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;

	return free_queue_push(&p_sniff_worker->tlist, p_node->data);
}

static bool minor_push_call(struct switch_queue_info *p_stat, struct supex_task_node *p_node, va_list *ap)
{
	void                    *user = va_arg(*ap, void *);
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	/*******************/
	char temp[32] = {};

	sprintf(temp, "%d_%d", p_sniff_worker->batch, p_sniff_worker->index);
	/*******************/

	return mq_store_put(temp, p_node->data, p_node->size);
}

/*pull*/
static bool major_pull_call(struct switch_queue_info *p_stat, struct supex_task_node *p_node, va_list *ap)
{
	void                    *user = va_arg(*ap, void *);
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;

	return free_queue_pull(&p_sniff_worker->tlist, p_node->data);
}

static bool minor_pull_call(struct switch_queue_info *p_stat, struct supex_task_node *p_node, va_list *ap)
{
	void                    *user = va_arg(*ap, void *);
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;

	/*******************/
	char temp[32] = {};

	sprintf(temp, "%d_%d", p_sniff_worker->batch, p_sniff_worker->index);
	/*******************/
	return mq_store_get(temp, p_node->data, p_node->size);
}

static bool sniff_task_report(void *user, void *task)
{
	SNIFF_WORKER_PTHREAD            *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	struct sniff_task_node          *p_task = (struct sniff_task_node *)task;
	struct switch_queue_info        *p_stat = &g_queue_stat_list[p_sniff_worker->batch * g_sniff_cfg_list.file_info.worker_counts + p_sniff_worker->index];

	p_stat->major_have = &p_sniff_worker->thave;
	p_stat->minor_have = &p_sniff_worker->thave;

	struct supex_task_node node;
	supex_node_init(&node, task, sizeof(struct sniff_task_node));
	return switch_queue_push(p_stat, &node, user);
}

static bool sniff_task_lookup(void *user, void *task)
{
	bool ok = false;

	SNIFF_WORKER_PTHREAD            *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	struct sniff_task_node          *p_task = (struct sniff_task_node *)task;
	struct switch_queue_info        *p_stat = &g_queue_stat_list[p_sniff_worker->batch * g_sniff_cfg_list.file_info.worker_counts + p_sniff_worker->index];

  #if 1
	AO_T have = AO_GET(&p_sniff_worker->thave);

	if ((have <= 0) && (p_stat->step_lookup == 2)) {
		ok = free_queue_pull(p_sniff_worker->glist, p_task);

		if (ok) {
			x_printf(D, "pull queue ok!");
		}

		return ok;
	}
  #endif
	p_stat->major_have = &p_sniff_worker->thave;
	p_stat->minor_have = &p_sniff_worker->thave;

	struct supex_task_node node;
	supex_node_init(&node, task, sizeof(struct sniff_task_node));
	return switch_queue_pull(p_stat, &node, user);
}
#endif	/* ifdef STORE_USE_UCMQ_AND_QUEUE */

static void main_entry_init(void)
{
#if defined(STORE_USE_UCMQ) || defined(STORE_USE_UCMQ_AND_QUEUE)
	bool ok = mq_store_init("./mq_data/logs", "./mq_data/data");

	if (!ok) {
		x_perror("mq_store_init");
		exit(EXIT_FAILURE);
	}

  #ifdef STORE_USE_UCMQ_AND_QUEUE
	int all = g_swift_cfg_list.file_info.worker_counts * g_sniff_cfg_list.file_info.worker_counts;
	g_queue_stat_list = calloc(all, sizeof(struct switch_queue_info));
	assert(g_queue_stat_list);

	while (all--) {
		struct switch_queue_info *p_stat = &g_queue_stat_list[all];
		switch_queue_init(p_stat, sizeof(struct sniff_task_node), NULL, major_push_call, major_pull_call,
			NULL, minor_push_call, minor_pull_call);
	}
  #endif
#else
  #ifdef STORE_USE_SHMQ
	g_tasks_shmqueue = SHM_QueueInit(0x00000001, MAX_LIMIT_FD, sizeof(struct sniff_task_node));

	assert(g_tasks_shmqueue);
  #endif
#endif	/* if defined(STORE_USE_UCMQ) || defined(STORE_USE_UCMQ_AND_QUEUE) */

	init_session_cmd();

	entry_init();
}

static void swift_shut_down()
{
	SWIFT_WORKER_PTHREAD    *swift_worker = g_swift_worker_pthread;
	const int               swift_worker_total = G_SWIFT_WORKER_COUNTS;
	int                     i = 0;
	int                     thds = 0;

	struct ThreadSuspend *cond = NULL;

	New(cond);

	if (!ThreadSuspendInit(cond)) {
		exit(EXIT_FAILURE);
	}

	for (i = 0; i < swift_worker_total; i++) {
		thds++;
		sniff_suspend_thread(swift_worker[i].mount, cond);
	}

	/*
	 * 等待所有sniff_worker挂起
	 */
	ThreadSuspendWait(cond, thds * G_SNIFF_WORKER_COUNTS);

	/*
	 * 由于 sniff_worker 线程还在挂起状态，所以不能释放挂起条件
	 */
}

static void *main_next_entry(void *arg)
{
	struct safe_init_step *info = arg;

	SAFE_PTHREAD_INIT_COME(info);

	SAFE_PTHREAD_INIT_OVER(info);

	swift_start();

	return NULL;
}

int main(int argc, char **argv)
{
	// ---> init smart
	load_supex_args(&g_smart_cfg_list.argv_info, argc, argv, "t:", NULL, NULL);

	load_smart_cfg_file(&g_smart_cfg_list.file_info, g_smart_cfg_list.argv_info.conf_name);

	// g_smart_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	// g_smart_cfg_list.func_info[APPLY_FUNC_ORDER].func = (SUPEX_TASK_CALLBACK)smart_vms_call;
	g_smart_cfg_list.func_info[FETCH_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_smart_cfg_list.func_info[FETCH_FUNC_ORDER].func = (SUPEX_TASK_CALLBACK)smart_vms_gain;
	g_smart_cfg_list.func_info[MERGE_FUNC_ORDER].type = BIT8_TASK_TYPE_WHOLE;
	g_smart_cfg_list.func_info[MERGE_FUNC_ORDER].func = (SUPEX_TASK_CALLBACK)smart_vms_sync;

	g_smart_cfg_list.vmsys_init = smart_vms_init;
	// g_smart_cfg_list.vmsys_cntl = smart_vms_cntl;
	// g_smart_cfg_list.vmsys_rfsh = smart_vms_rfsh;
	g_smart_cfg_list.vmsys_monitor = smart_vms_monitor;

	smart_mount(&g_smart_cfg_list);

	// ---> init swift
	snprintf(g_swift_cfg_list.argv_info.conf_name,
		sizeof(g_swift_cfg_list.argv_info.conf_name),
		"%s", g_smart_cfg_list.argv_info.conf_name);

	snprintf(g_swift_cfg_list.argv_info.serv_name,
		sizeof(g_swift_cfg_list.argv_info.serv_name),
		"%s", g_smart_cfg_list.argv_info.serv_name);

	snprintf(g_swift_cfg_list.argv_info.msmq_name,
		sizeof(g_swift_cfg_list.argv_info.msmq_name),
		"%s", g_smart_cfg_list.argv_info.msmq_name);

	bool ok = load_swift_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	if (!ok) {
		exit(EXIT_FAILURE);
	}

	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_CALLBACK)swift_vms_call;
	// g_swift_cfg_list.func_info[ FETCH_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	// g_swift_cfg_list.func_info[ FETCH_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_gain;
	// g_swift_cfg_list.func_info[ MERGE_FUNC_ORDER ].type = BIT8_TASK_TYPE_WHOLE;
	// g_swift_cfg_list.func_info[ MERGE_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_sync;
	// g_swift_cfg_list.func_info[CUSTOM_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	// g_swift_cfg_list.func_info[CUSTOM_FUNC_ORDER].func = (TASK_CALLBACK)swift_vms_exec;

	g_swift_cfg_list.pthrd_init = swift_pthrd_init;

	g_swift_cfg_list.shut_down = swift_shut_down;

	g_swift_cfg_list.vmsys_init = swift_vms_init;
	// g_swift_cfg_list.vmsys_exit = swift_vms_exit;
	// g_swift_cfg_list.vmsys_cntl = swift_vms_cntl;
	// g_swift_cfg_list.vmsys_rfsh = swift_vms_rfsh;

	swift_mount(&g_swift_cfg_list);

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

	/*
	 *        g_sniff_cfg_list.func_info[ APPLY_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	 *        g_sniff_cfg_list.func_info[ APPLY_FUNC_ORDER ].func = (TASK_CALLBACK)sniff_vms_call;
	 */
	g_sniff_cfg_list.task_lookup = sniff_task_lookup;
	g_sniff_cfg_list.task_report = sniff_task_report;

	g_sniff_cfg_list.vmsys_init = sniff_vms_init;
	g_sniff_cfg_list.vmsys_exit = sniff_vms_exit;
	// g_sniff_cfg_list.vmsys_cntl = sniff_vms_cntl;
	// g_sniff_cfg_list.vmsys_rfsh = sniff_vms_rfsh;

	sniff_mount(&g_sniff_cfg_list);

	// ---> all ok
	main_entry_init();

	safe_start_pthread((void *)main_next_entry, 1, NULL, NULL);

	smart_start();
	return 0;
}

