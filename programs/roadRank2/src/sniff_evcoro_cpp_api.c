#include <assert.h>
#include <unistd.h>

#include "rr_cfg.h"
#include "utils.h"

#include "minor/sniff_api.h"
#include "sniff_evcoro_cpp_api.h"
#include "tcp_api/tcp_api.h"
#include "async_tasks/async_api.h"
#include "pool_api/conn_xpool_api.h"
#include "base/utils.h"
#include "redis_api/redis_status.h"

#include "evcoro_async_tasks.h"
#include "spx_evcs.h"

#include "rr_handle.h"

#define REDIS_ERR               -1
#define REDIS_OK                0
#define OVERLOOK_DELAY_LIMIT    3

extern struct rr_cfg_file g_rr_cfg_file;
int sniff_vms_call(void *user, union virtual_system **VMS, struct sniff_task_node *task)
{
	struct sniff_task_node  *p_task = task;
	time_t                  delay = time(NULL) - p_task->stamp;
	//	struct supex_evcoro     *p_evcoro = supex_get_default();
	//	struct ev_loop          *loop = p_evcoro->scheduler->listener;
	struct ev_loop *loop = supex_get_default()->scheduler->listener;

	x_printf(I, "task <shift> %d\t<come> %ld\t<delay> %ld",
		p_task->base.shift, p_task->stamp, delay);

	x_printf(D, "%s", p_task->data);

	if (delay >= OVERLOOK_DELAY_LIMIT) {
		x_printf(W, "overlook one task");
		return -1;
	}

	int ret = rr_task_handle(loop, p_task->data);

	if (ret < 0) {
		return -1;
	}

	return 0;
}

