#pragma once
#include <ev.h>

#include "share_evcb.h"
#include "evmdl.h"


struct session_module {
	struct LineBuff rlbuff;
	char            fifopath[MAX_FILE_PATH_SIZE];
	struct ev_io            session_watcher;	/* accept command from fifo*/
};

struct mqueue_module {
	struct ev_io            msmq_watcher;	/* msmq watcher for new message */
	void			(*usr_mqueue_fcb)(const char *data);
};


typedef struct
{

	unsigned int            robin;		/**< 选取接收线程的下标HIT*///TODO move
	unsigned int            robout;		/**< 选取发送线程的下标HIT*/

	pthread_t               thread_id;	/* unique ID of this thread */
	long                    tid;

	struct ev_loop          *loop;			/* libev loop this thread uses */
	struct monitor_module   mdl_monitor;
	struct session_module   mdl_session;
	struct accept_module 	mdl_accept;
	struct update_module    mdl_update;
	struct reload_module    mdl_reload;
	struct mqueue_module 	mdl_mqueue;

	struct signal_module	mdl_sigquit;
	struct signal_module	mdl_sigpipe;
	struct signal_module	mdl_sigint;
} LISTEN_PTHREAD;
