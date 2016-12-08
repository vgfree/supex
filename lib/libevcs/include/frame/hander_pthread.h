#pragma once
#include "engine/evmdl.h"

#include "engine/proto_comm.h"
#include "engine/supex.h"
#include "engine/adopt_tasks/adopt_task.h"
#include "engine/spx/spx_evcs.h"
#include "engine/base/free_queue.h"

typedef struct
{
	int                     index;		/**< 所属接收线程池下标*/
	unsigned int            robin;		/**< 选取工作线程的下标HIT*/
#ifdef OPEN_EQUAL
	int                     work_num;	/* 任务数量 */
#endif
	int                     ptype;

	long                    tid;
	pthread_t               pid;	/**< 所属线程 unique ID of this thread */
	struct ev_loop          *loop;	/**< 该句柄的事件侦听器 libev loop this thread uses */
#ifdef USE_PIPE
	struct pipe_module      mdl_recv_pipe;
	struct pipe_module      mdl_send_pipe;
#else
	struct list_module      mdl_recv_list;
	struct list_module      mdl_send_list;
#endif
#ifdef OPEN_EVCORO
	struct supex_evcoro     *p_evcs;
#endif
	void                    *mount;
} HANDER_PTHREAD;

