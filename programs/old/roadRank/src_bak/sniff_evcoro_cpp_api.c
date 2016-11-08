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

#include "decode_gps.h"
#include "match_road.h"
#include "calculate.h"

#define REDIS_ERR       -1
#define REDIS_OK        0
#define OVERLOOK_DELAY_LIMIT 3

extern struct rr_cfg_file g_rr_cfg_file;
int sniff_vms_call(void *user, union virtual_system **VMS, struct sniff_task_node *task)
{
	struct sniff_task_node  *p_task = task;
	time_t                  delay = time(NULL) - p_task->stamp;
//	struct supex_evcoro     *p_evcoro = supex_get_default();
//	struct ev_loop          *loop = p_evcoro->scheduler->listener;
        struct ev_loop  *loop = supex_get_default()->scheduler->listener;
	x_printf(I, "task <shift> %d\t<come> %ld\t<delay> %ld",
		p_task->base.shift, p_task->stamp, delay);

	x_printf(D, "%s", p_task->data);

	if (delay >= OVERLOOK_DELAY_LIMIT) {
		x_printf(W, "overlook one task");
		return -1;
	}

	CAL_INFO *cal_info = (CAL_INFO *)calloc(1, sizeof(CAL_INFO));
	assert(cal_info);
	memset(cal_info, 0, sizeof(CAL_INFO));
	int ok = gps_decode(loop, p_task->data, &(cal_info->gps_info));

	if (ok == -1) {
		free(cal_info); cal_info = NULL;
		return -1;
	}

	// finish callback

	cal_info->cal_callback = calculate;
	ok = match_road_v2(loop, cal_info);

	if (ok == -1) {
		x_printf(E, "Failed match road!");
		free(cal_info); cal_info = NULL;
		return -1;
	}
        
        if(cal_info)        
                free(cal_info);
	return 0;
}

