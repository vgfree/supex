#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "mq_api.h"

#include "mfptp_api.h"
#include "mfptp_cpp_api.h"
#include "load_mfptp_cfg.h"
#include "zmq_weibo_forward.h"

#include "route_cfg.h"
#include "user_topo.h"
#include "sqlite_api.h"
#include "x_utils.h"

extern struct file_log_s g_file_log;	// 日志结构

struct mfptp_cfg_list   g_mfptp_cfg_list = {};
struct route_cfg_file   g_route_cfg_file = {};

static void mfptp_pthrd_init(void *user)
{}

#ifdef STORE_USE_QUEUE
static bool mfptp_task_report(void *user, void *task)
{
	return supex_task_push(&((MFPTP_WORKER_PTHREAD *)user)->tlist, task);
}

static bool mfptp_task_lookup(void *user, void *task)
{
	return supex_task_pull(&((MFPTP_WORKER_PTHREAD *)user)->tlist, task);
}
#endif

#ifdef STORE_USE_UCMQ
static bool mfptp_task_report(void *user, void *task)
{
	MFPTP_WORKER_PTHREAD    *p_worker = (MFPTP_WORKER_PTHREAD *)user;
	char                    temp[32] = {};

	sprintf(temp, "%d", p_worker->index);
	bool ok = mq_store_put(temp, task, sizeof(struct mfptp_task_node));

	if (!ok) {
		x_printf(E, "push failed!\n");
	} else {
		x_printf(D, "push ok!\n");
		// struct mfptp_task_node *p_task = task;
		// printf("function %p\n", p_task->func);
	}

	return ok;
}

static bool mfptp_task_lookup(void *user, void *task)
{
	MFPTP_WORKER_PTHREAD    *p_worker = (MFPTP_WORKER_PTHREAD *)user;
	char                    temp[32] = {};

	sprintf(temp, "%d", p_worker->index);
	bool ok = mq_store_get(temp, task, sizeof(struct mfptp_task_node));

	if (!ok) {
		// x_printf(D, "pull NULL!\n");
	} else {
		x_printf(D, "pull ok!\n");
		// struct mfptp_task_node *p_task = task;
		// printf("function %p\n", p_task->func);
	}

	return ok;
}
#endif	/* ifdef STORE_USE_UCMQ */
#ifdef STORE_USE_UCMQ_AND_QUEUE
  #define MAX_MFPTP_TEMP_QUEUE_NUMBER 2		// must >= 2
enum
{
	MARK_USE_QUEUE = 0,
	MARK_USE_UCMQ,
};
struct queue_stat_info
{
	int                     mark_report;
	unsigned int            shift_report;
	int                     mark_lookup;
	unsigned int            shift_lookup;
	int                     step_lookup;
	struct mfptp_task_node  temp;
	struct supex_task_list  swap;
};

static struct queue_stat_info *g_queue_stat_list = NULL;

static bool mfptp_task_report(void *user, void *task)
{
	bool                    ok = false;
	int                     new_mark = 0;
	unsigned int            new_shift = 0;
	MFPTP_WORKER_PTHREAD    *p_worker = (MFPTP_WORKER_PTHREAD *)user;
	struct mfptp_task_node  *p_task = (struct mfptp_task_node *)task;
	struct queue_stat_info  *p_stat = &g_queue_stat_list[g_mfptp_cfg_list.file_info.worker_counts + p_worker->index];

	do {
		// ---->use queue
		new_mark = MARK_USE_QUEUE;

		if (p_stat->mark_report == MARK_USE_QUEUE) {
			p_task->shift = p_stat->shift_report;
		} else {
			p_task->shift = p_stat->shift_report + 1;
		}

		new_shift = p_task->shift;
		ok = supex_task_push(&p_worker->tlist, p_task);

		if (ok) {
			x_printf(D, "push queue ok!\n");
			LOG(LOG_NET_DOWNLINK, D, "push queue ok!\n");
			p_stat->mark_report = new_mark;
			p_stat->shift_report = new_shift;
			__sync_add_and_fetch(&p_worker->thave, 1);
			break;
		} else {
			x_printf(W, "push queue failed!\n");
			LOG(LOG_NET_DOWNLINK, W, "push queue failed!\n");
		}

		/*******************/
		char temp[32] = {};
		sprintf(temp, "%d", p_worker->index);
		/*******************/

		// ---->use ucmq
		new_mark = MARK_USE_UCMQ;

		if (p_stat->mark_report == MARK_USE_UCMQ) {
			p_task->shift = p_stat->shift_report;
		} else {
			p_task->shift = p_stat->shift_report + 1;
		}

		new_shift = p_task->shift;
		ok = mq_store_put(temp, task, sizeof(struct mfptp_task_node));

		if (ok) {
			x_printf(D, "push ucmq ok!\n");
			LOG(LOG_NET_DOWNLINK, D, "push ucmq ok!\n");
			p_stat->mark_report = new_mark;
			p_stat->shift_report = new_shift;
			__sync_add_and_fetch(&p_worker->thave, 1);
			break;
		} else {
			x_printf(E, "push ucmq failed!\n");
			LOG(LOG_NET_DOWNLINK, E, "push ucmq failed!\n");
		}
	} while (0);

	return ok;
}

static bool mfptp_task_lookup(void *user, void *task)
{
	char                    temp[32] = {};
	bool                    ok = false;
	int                     have = 0;
	MFPTP_WORKER_PTHREAD    *p_worker = (MFPTP_WORKER_PTHREAD *)user;
	struct mfptp_task_node  *p_task = (struct mfptp_task_node *)task;
	struct queue_stat_info  *p_stat = &g_queue_stat_list[g_mfptp_cfg_list.file_info.worker_counts + p_worker->index];

	have = p_worker->thave;

	if ((have <= 0) && (p_stat->step_lookup == 2)) {
		return supex_task_pull(p_worker->glist, p_task);
	}

	switch (p_stat->step_lookup)
	{
		case 0:
			/*init vms step*/
			ok = supex_task_pull(&p_worker->tlist, p_task);

			if (ok) {
				__sync_sub_and_fetch(&p_worker->thave, 1);
				p_stat->step_lookup++;
			}

			break;

		case 1:
			/*done old step*/
			/*******************/
			sprintf(temp, "%d", p_worker->index);
			/*******************/
			ok = mq_store_get(temp, task, sizeof(struct mfptp_task_node));

			if (ok) {
				if (p_task->shift == 1) {
					supex_task_push(&p_stat->swap, p_task);	// push l
					p_stat->step_lookup++;
				} else {
					// do nothing, is old task
				}
			} else {
				p_stat->step_lookup++;
			}

			break;

		case 2:

			/*done new step*/
			if (p_stat->mark_lookup == MARK_USE_QUEUE) {
				ok = supex_task_pull(&p_worker->tlist, p_task);

				if (ok) {
					if (p_task->shift == p_stat->shift_lookup) {
						// do nothing, is next task
						__sync_sub_and_fetch(&p_worker->thave, 1);
					} else {
						/*not first push then pop,it will get the push task in one loop.*/
						memcpy(&p_stat->temp, p_task, sizeof(struct mfptp_task_node));
						ok = supex_task_pull(&p_stat->swap, p_task);	// pull l

						if (ok) {
							__sync_sub_and_fetch(&p_worker->thave, 1);
						}

						supex_task_push(&p_stat->swap, &p_stat->temp);	// push l
						p_stat->shift_lookup++;
						p_stat->mark_lookup = MARK_USE_UCMQ;
					}
				} else {
					if (have > 0) {
						ok = supex_task_pull(&p_stat->swap, p_task);	// pull l

						if (ok) {
							__sync_sub_and_fetch(&p_worker->thave, 1);
						}

						p_stat->shift_lookup++;
						p_stat->mark_lookup = MARK_USE_UCMQ;
					} else {
						// do nothing, no task
					}
				}
			} else {
				/*******************/
				sprintf(temp, "%d", p_worker->index);
				/*******************/
				ok = mq_store_get(temp, task, sizeof(struct mfptp_task_node));

				if (ok) {
					if (p_task->shift == p_stat->shift_lookup) {
						// do nothing, is next task
						__sync_sub_and_fetch(&p_worker->thave, 1);
					} else {
						memcpy(&p_stat->temp, p_task, sizeof(struct mfptp_task_node));
						ok = supex_task_pull(&p_stat->swap, p_task);	// pull l

						if (ok) {
							__sync_sub_and_fetch(&p_worker->thave, 1);
						}

						supex_task_push(&p_stat->swap, &p_stat->temp);	// push l
						p_stat->shift_lookup++;
						p_stat->mark_lookup = MARK_USE_QUEUE;
					}
				} else {
					if (have > 0) {
						ok = supex_task_pull(&p_stat->swap, p_task);	// pull l

						if (ok) {
							__sync_sub_and_fetch(&p_worker->thave, 1);
						}

						p_stat->shift_lookup++;
						p_stat->mark_lookup = MARK_USE_QUEUE;
					} else {
						// do nothing, no task
					}
				}
			}

			break;

		default:
			break;
	}
	return ok;
}
#endif	/* ifdef STORE_USE_UCMQ_AND_QUEUE */

void mfptp_entry_init(void)
{
	struct sql_info info;

	info.database = "testdb.sqlite";
	info.table_name = "user_channel";
	pthread_t       usr_weibo_id;
	pthread_t       gp_weibo_id;

	/* 初始化 SQLITE 数据库 */
	topo_data_mem_init();
	topo_load_user_record(&info);

#if defined(STORE_USE_UCMQ) || defined(STORE_USE_UCMQ_AND_QUEUE)
	bool ok = mq_store_init("./mq_data/logs", "./mq_data/data");

	if (!ok) {
		x_perror("mq_store_init error\n");
		LOG(LOG_INIT, E, "mq_store_init error\n");
		exit(EXIT_FAILURE);
	}

  #ifdef STORE_USE_UCMQ_AND_QUEUE
	int all = g_mfptp_cfg_list.file_info.worker_counts;
	g_queue_stat_list = calloc(all, sizeof(struct queue_stat_info));
	assert(g_queue_stat_list);
	struct queue_stat_info *p_stat = NULL;

	while (all--) {
		p_stat = &g_queue_stat_list[all];
		supex_task_init(&p_stat->swap, sizeof(struct mfptp_task_node), MAX_MFPTP_TEMP_QUEUE_NUMBER);
	}
  #endif
#endif
	pthread_create(&usr_weibo_id, NULL, mfptp_usr_weibo_forward, NULL);
	pthread_create(&gp_weibo_id, NULL, mfptp_gp_weibo_forward, NULL);
	LOG(LOG_INIT, D, "mfptpServer entry init ,OK!\n");
}

int mfptp_vmsys_init(void *W)
{
	return 0;
}

int main(int argc, char **argv)
{
	log_start();
	// ---> init mfptp
	load_mfptp_cfg_argv(&g_mfptp_cfg_list.argv_info, argc, argv);

	load_mfptp_cfg_file(&g_mfptp_cfg_list.file_info, g_mfptp_cfg_list.argv_info.conf_name);

	g_mfptp_cfg_list.entry_init = mfptp_entry_init;
	g_mfptp_cfg_list.pthrd_init = mfptp_pthrd_init;
	g_mfptp_cfg_list.vmsys_init = mfptp_vmsys_init;

	g_mfptp_cfg_list.task_lookup = mfptp_task_lookup;
	g_mfptp_cfg_list.task_report = mfptp_task_report;

	g_mfptp_cfg_list.drift_away = mfptp_drift_away;
	g_mfptp_cfg_list.drift_come = mfptp_drift_come;

	// ---> init route
	read_route_cfg(&g_route_cfg_file, g_mfptp_cfg_list.argv_info.conf_name);

	mfptp_mount(&g_mfptp_cfg_list);

	mfptp_start();
	return 0;
}

