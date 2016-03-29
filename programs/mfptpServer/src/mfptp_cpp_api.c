#include <string.h>
#include <stdlib.h>

#include "x_utils.h"
#include "mfptp_api.h"
#include "mfptp_cpp_api.h"

extern struct mfptp_cfg_list g_mfptp_cfg_list;

int mfptp_drift_away(void *W)
{
	MFPTP_WORKER_PTHREAD *p_worker = (MFPTP_WORKER_PTHREAD *)W;

	// struct user_info *p_info = xxxx;
	// TODO 组装zmq消息,并且转发数据

	return 0;
}

int mfptp_drift_come(void)
{
	//	supex_evuv_wake( &p_worker->evuv );
	return 0;
}

