#include "ev.h"

#include "timport_cfg.h"

struct timport_settings
{
	struct timport_cfg_list *conf;
};

typedef struct
{
	pthread_t               thread_id;		/* unique ID of this thread */

	struct ev_loop          *loop;			/* libev loop this thread uses */
	struct ev_signal        sigquit_watcher;
	struct ev_signal        sigint_watcher;
	struct ev_signal        sigpipe_watcher;
	struct ev_timer         timer_watcher;
} TIMPORT_MASTER_PTHREAD;

int timport_mount(struct timport_cfg_list *conf);

int timport_start(void);

