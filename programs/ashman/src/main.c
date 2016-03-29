#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "mq_api.h"
// #include "luakvutils.h"

#include "swift_api.h"
#include "swift_cpp_api.h"
#include "load_swift_cfg.h"

#include "sniff_api.h"
#include "load_sniff_cfg.h"
#include "switch_queue.h"

#ifdef OPEN_SCCO
  #include "sniff_scco_lua_api.h"
#else
  #include "sniff_line_lua_api.h"
#endif

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
	return supex_task_push(&((SNIFF_WORKER_PTHREAD *)user)->tlist, task);
}

static bool sniff_task_lookup(void *user, void *task)
{
	return supex_task_pull(&((SNIFF_WORKER_PTHREAD *)user)->tlist, task);
}
#endif

#ifdef STORE_USE_UCMQ
static bool sniff_task_report(void *user, void *task)
{
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	char                    temp[32] = {};

	sprintf(temp, "%d_%d", p_sniff_worker->batch, p_sniff_worker->index);
	bool ok = mq_store_put(temp, task, sizeof(struct sniff_task_node));

	if (!ok) {
		x_printf(E, "push failed!");
	} else {
		x_printf(D, "push ok!");
		// struct sniff_task_node *p_task = task;
		// printf("function %p\n", p_task->func);
	}

	return ok;
}

static bool sniff_task_lookup(void *user, void *task)
{
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	char                    temp[32] = {};

	sprintf(temp, "%d_%d", p_sniff_worker->batch, p_sniff_worker->index);
	bool ok = mq_store_get(temp, task, sizeof(struct sniff_task_node));

	if (!ok) {
		// x_printf(D, "pull NULL!");
	} else {
		x_printf(D, "pull ok!");
		// struct sniff_task_node *p_task = task;
		// printf("function %p\n", p_task->func);
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

	return supex_task_push(&p_sniff_worker->tlist, p_node->data);
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

	return supex_task_pull(&p_sniff_worker->tlist, p_node->data);
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
	AO_T have = ATOMIC_GET(&p_sniff_worker->thave);

	if ((have <= 0) && (p_stat->step_lookup == 2)) {
		ok = supex_task_pull(p_sniff_worker->glist, p_task);

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

void swift_entry_init(void)
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
#endif

	// if (kv_init(NULL) != ERR_NONE) {
	//      fprintf(stdout, "libaray key-value initialize failed");
	//      exit(-1);
	// }
	if (!kvpool_init()) {
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	// ---> init swift
	load_supex_args(&g_swift_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	load_swift_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_CALLBACK)swift_vms_call;
	// g_swift_cfg_list.func_info[ FETCH_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	// g_swift_cfg_list.func_info[ FETCH_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_gain;
	// g_swift_cfg_list.func_info[ MERGE_FUNC_ORDER ].type = BIT8_TASK_TYPE_WHOLE;
	// g_swift_cfg_list.func_info[ MERGE_FUNC_ORDER ].func = (TASK_CALLBACK)swift_vms_sync;
	g_swift_cfg_list.func_info[CUSTOM_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_swift_cfg_list.func_info[CUSTOM_FUNC_ORDER].func = (TASK_CALLBACK)swift_vms_exec;

	g_swift_cfg_list.entry_init = swift_entry_init;
	g_swift_cfg_list.pthrd_init = swift_pthrd_init;

	g_swift_cfg_list.vmsys_init = swift_vms_init;
	// g_swift_cfg_list.vmsys_exit = swift_vms_exit;
	// g_swift_cfg_list.vmsys_cntl = swift_vms_cntl;
	// g_swift_cfg_list.vmsys_rfsh = swift_vms_rfsh;
#ifdef STORE_USE_UCMQ_AND_QUEUE
	// g_swift_cfg_list.vmsys_idle = swift_vms_idle;
	/*have bug when tasks pile up*/
#endif

	swift_mount(&g_swift_cfg_list);

	// ---> init sniff
	memcpy(g_sniff_cfg_list.argv_info.conf_name,
		g_swift_cfg_list.argv_info.conf_name, MAX_FILE_NAME_SIZE);
	memcpy(g_sniff_cfg_list.argv_info.serv_name,
		g_swift_cfg_list.argv_info.serv_name, MAX_FILE_NAME_SIZE);
	memcpy(g_sniff_cfg_list.argv_info.msmq_name,
		g_swift_cfg_list.argv_info.msmq_name, MAX_FILE_NAME_SIZE);

	load_sniff_cfg_file(&g_sniff_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	/*
	 *        g_sniff_cfg_list.func_info[ APPLY_FUNC_ORDER ].type = BIT8_TASK_TYPE_ALONE;
	 *        g_sniff_cfg_list.func_info[ APPLY_FUNC_ORDER ].func = (TASK_CALLBACK)sniff_vms_call;
	 */
	g_sniff_cfg_list.task_lookup = sniff_task_lookup;
	g_sniff_cfg_list.task_report = sniff_task_report;

	g_sniff_cfg_list.vmsys_init = sniff_vms_init;

	sniff_mount(&g_sniff_cfg_list);

	swift_start();
	return 0;
}

