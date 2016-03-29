#include <pthread.h>

#include "net_cache.h"
#include "utils.h"

#include "timport_api.h"
#include "timport_evcb.h"

struct timport_settings g_timport_settings = {};

TIMPORT_MASTER_PTHREAD g_timport_master_pthread = {};

static struct safe_once_init    g_timport_mount_mark = { 0, NULLOBJ_AO_SPINLOCK };
static struct safe_once_init    g_first_init_mark = { 0, NULLOBJ_AO_SPINLOCK };

static void cfg_check(struct timport_cfg_list *conf)
{
	(void)conf;
}

static void first_init(void)
{
	SAFE_ONCE_INIT_COME(&g_first_init_mark);

	BindCPUCore(-1);

	struct timport_cfg_file *p_cfg_file = &(g_timport_settings.conf->file_info);

	cache_peak((unsigned)g_timport_settings.conf->file_info.max_req_size);

	char path[MAX_PATH_SIZE] = {};

	snprintf(path, sizeof(path), "%s/%s.log", p_cfg_file->log_path, p_cfg_file->log_file);

	SLogOpen(path, SLogIntegerToLevel(p_cfg_file->log_level));

	SAFE_ONCE_INIT_OVER(&g_first_init_mark);
}

int timport_mount(struct timport_cfg_list *conf)
{
	SAFE_ONCE_INIT_COME(&g_timport_mount_mark);

	cfg_check(conf);
	g_timport_settings.conf = conf;

	SAFE_ONCE_INIT_OVER(&g_timport_mount_mark);
	return 0;
}

int timport_start(void)
{
	struct timport_cfg_list *conf = NULL;
	struct supex_argv       *argv_info = NULL;

	/* init data */
	first_init();

	conf = g_timport_settings.conf;
	argv_info = &conf->argv_info;

	if (conf->entry_init) {
		conf->entry_init();
	}

	g_timport_master_pthread.thread_id = pthread_self();

	/* set loop */
	g_timport_master_pthread.loop = ev_default_loop(0);

	/* register signal */
	ev_signal_init(&(g_timport_master_pthread.sigquit_watcher), timport_signal_cb, SIGQUIT);
	ev_signal_start(g_timport_master_pthread.loop, &(g_timport_master_pthread.sigquit_watcher));

	ev_signal_init(&(g_timport_master_pthread.sigint_watcher), timport_signal_cb, SIGINT);
	ev_signal_start(g_timport_master_pthread.loop, &(g_timport_master_pthread.sigint_watcher));

	ev_signal_init(&(g_timport_master_pthread.sigpipe_watcher), timport_signal_cb, SIGPIPE);
	ev_signal_start(g_timport_master_pthread.loop, &(g_timport_master_pthread.sigpipe_watcher));

	/* set timer */
	g_timport_master_pthread.timer_watcher.data = (void *)&g_timport_master_pthread;
	ev_timer_init(&(g_timport_master_pthread.timer_watcher), timport_timer_cb, 0., 0.);
	ev_timer_start(g_timport_master_pthread.loop, &(g_timport_master_pthread.timer_watcher));

	ev_loop(g_timport_master_pthread.loop, 0);
	ev_loop_destroy(g_timport_master_pthread.loop);

	if (conf->shut_down) {
		conf->shut_down();
	}

	return 0;
}

