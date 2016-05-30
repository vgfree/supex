#include "switch_queue.h"
#include "app_queue.h"

extern struct swift_cfg_list   g_swift_cfg_list;
extern struct sniff_cfg_list   g_sniff_cfg_list;

#ifdef STORE_USE_QUEUE
bool sniff_task_report(void *user, void *task)
{
	bool ok = false;

	ok = supex_task_push(&((SNIFF_WORKER_PTHREAD *)user)->tlist, task);

	if (ok) {
		x_printf(D, "push queue ok!");
		ATOMIC_INC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
	} else {
		x_printf(E, "push queue fail!");
	}

	return ok;
}

bool sniff_task_lookup(void *user, void *task)
{
	bool ok = false;

	ok = supex_task_pull(&((SNIFF_WORKER_PTHREAD *)user)->tlist, task);

	if (ok) {
		x_printf(D, "pull queue ok!");
		ATOMIC_DEC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
	}

	return ok;
}
#endif	/* ifdef STORE_USE_QUEUE */

#if defined(STORE_USE_SHMQ)

ShmQueueT g_tasks_shmqueue = NULL;

bool sniff_task_report(void *user, void *task)
{
	bool ok = false;

	//        ok = supex_task_push(&((SNIFF_WORKER_PTHREAD *)user)->tlist, task);

	ok = SHM_QueuePush(g_tasks_shmqueue, task, sizeof(struct sniff_task_node), NULL);

	if (ok) {
		x_printf(D, "push queue ok!");
		ATOMIC_INC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
	} else {
		x_printf(E, "push queue fail!");
	}

	return ok;
}

bool sniff_task_lookup(void *user, void *task)
{
	bool ok = false;

	//        ok = supex_task_pull(&((SNIFF_WORKER_PTHREAD *)user)->tlist, task);

	ok = SHM_QueuePull(g_tasks_shmqueue, task, sizeof(struct sniff_task_node), NULL);

	if (ok) {
		x_printf(D, "pull queue ok!");
		ATOMIC_DEC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
	}

	return ok;
}
#endif	/* if defined(STORE_USE_SHMQ) */

#ifdef STORE_USE_UCMQ
bool sniff_task_report(void *user, void *task)
{
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	char                    temp[32] = {};

	sprintf(temp, "%d_%d", p_sniff_worker->batch, p_sniff_worker->index);
	bool ok = mq_store_put(temp, task, sizeof(struct sniff_task_node));

	if (ok) {
		ATOMIC_INC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
		x_printf(D, "push queue ok!");
	} else {
		x_printf(E, "push queue fail!");
	}

	return ok;
}

bool sniff_task_lookup(void *user, void *task)
{
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	char                    temp[32] = {};

	sprintf(temp, "%d_%d", p_sniff_worker->batch, p_sniff_worker->index);
	bool ok = mq_store_get(temp, task, sizeof(struct sniff_task_node));

	if (ok) {
		ATOMIC_DEC(&((SNIFF_WORKER_PTHREAD *)user)->thave);
		x_printf(D, "pull queue ok!");
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

bool sniff_task_report(void *user, void *task)
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

bool sniff_task_lookup(void *user, void *task)
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

void app_queue_init(void)
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
	g_tasks_shmqueue = SHM_QueueInit(0x00000001, MAX_LIMIT_FD, sizeof(struct sniff_task_node));

	assert(g_tasks_shmqueue);
#endif	/* if defined(STORE_USE_UCMQ) || defined(STORE_USE_UCMQ_AND_QUEUE) */
}
