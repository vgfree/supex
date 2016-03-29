#pragma once

#include <ev.h>
#include <arpa/inet.h>
#include <mqueue.h>

#include "list.h"
#include "utils.h"
#include "supex.h"
#include "sniff_cfg.h"
#include "sniff_task.h"

struct sniff_settings
{
	struct sniff_cfg_list *conf;
};

typedef struct sniff_worker_pthread
{
#ifdef OPEN_SCCO
	struct supex_scco               scco;
	int                             airing;	/*BIT8_TASK_TYPE_WHOLE task count*/
#endif
#ifdef OPEN_LINE
	struct supex_line               line;
#endif
#ifdef OPEN_EVUV
	struct supex_evuv               evuv;
#endif
	void                            *user;			/**< */

	void                            *data;			/**< 所属swift的指针*/
	int                             batch;			/**< 创造的批次*/
	int                             genus;			/*延迟路由、非延迟路由的*/

	int                             index;
	pthread_t                       thread_id;		/* unique ID of this thread */
	long                            tid;
	AO_T                            thave;			/*can't use unsigned int*/
	struct supex_task_list          tlist;
	struct supex_task_list          *glist;
	struct sniff_worker_pthread     *next;
} SNIFF_WORKER_PTHREAD;

extern int SNIFF_WORKER_COUNTS;
/* -------------                             */

int sniff_mount(struct sniff_cfg_list *conf);

SNIFF_WORKER_PTHREAD *sniff_start(void *data, int batch, int genus);

typedef void (*SNIFF_VMS_ERR)(void **base);
#ifdef OPEN_SCCO
typedef int (*SNIFF_VMS_FCB)(void **base, int last, struct sniff_task_node *task, long S);

int sniff_for_alone_vm(void *user, void *task, int step, SNIFF_VMS_FCB vms_fcb, SNIFF_VMS_ERR vms_err);

int sniff_for_batch_vm(void *user, void *task, int step, SNIFF_VMS_FCB vms_fcb, SNIFF_VMS_ERR vms_err);
#endif
#if defined(OPEN_LINE) || defined(OPEN_EVUV)
typedef int (*SNIFF_VMS_FCB)(void **base, int last, struct sniff_task_node *task);

int sniff_for_alone_vm(void *user, void *task, SNIFF_VMS_FCB vms_fcb, SNIFF_VMS_ERR vms_err);
#endif

void sniff_all_task_hit(SNIFF_WORKER_PTHREAD *head, struct sniff_task_node *task);

void sniff_one_task_hit(SNIFF_WORKER_PTHREAD *head, struct sniff_task_node *task);

bool sniff_suspend_thread(SNIFF_WORKER_PTHREAD *sniff_worker, struct ThreadSuspend *cond);

