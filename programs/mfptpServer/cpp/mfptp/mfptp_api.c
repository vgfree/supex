#include <fcntl.h>	/* Obtain O_* constant definitions */
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>		/* For mode constants */
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "mfptp_api.h"
#include "mfptp_evcb.h"
#include "share_evcb.h"
#include "net_cache.h"
#include "zmqskt.h"
#include "zmq_weibo_forward.h"
#include "limits.h"
#include "mfptp_users_rbtree.h"
#include "user_topo.h"

int             G_WORKER_COUNTS = 0;
int             g_mfptp_init_done = false;
extern rb_root  user_tree;
/******************************open************************************/
struct mfptp_settings g_mfptp_settings = {};

MASTER_PTHREAD                  g_master_pthread = {};
MFPTP_WORKER_PTHREAD            *g_worker_pthread = NULL;
static struct safe_once_init    g_mfptp_mount_mark = { 0, NULLOBJ_AO_SPINLOCK };
static struct safe_once_init    g_first_init_mark = { 0, NULLOBJ_AO_SPINLOCK };
/****************************function**********************************/
static void cfg_check(struct mfptp_cfg_list *conf)
{
	assert(conf->file_info.worker_counts);
}

static void first_init(void)
{
	SAFE_ONCE_INIT_COME(&g_first_init_mark);

	struct mfptp_cfg_file *p_cfg_file = &(g_mfptp_settings.conf->file_info);
	G_WORKER_COUNTS = p_cfg_file->worker_counts;

	cache_peak(g_mfptp_settings.conf->file_info.max_req_size);

	init_log(p_cfg_file->log_path, p_cfg_file->log_file, p_cfg_file->log_level);
	open_new_log();

	SAFE_ONCE_INIT_OVER(&g_first_init_mark);
}

/*================================================================================*/
int mfptp_msg_pour(void *user, void *task)
{
	struct mfptp_task_node  *p_task = task;
	MFPTP_WORKER_PTHREAD    *p_worker = (MFPTP_WORKER_PTHREAD *)user;
	time_t                  delay = time(NULL) - p_task->stamp;

	x_printf(S, "channel %d\t|task <shift> %d\t<come> %d\t<delay> %d\n",
		p_worker->index, p_task->shift, p_task->stamp, delay);
	return 0;
}

static void mfptp_find_usr_cb(USER_RECORD *ur)
{
	pthread_t cur = pthread_self();

	if (NULL == ur) {
		x_printf(D, "出错啦，用户竟然是空的\n");
		LOG(LOG_NET_DOWNLINK, D, "error ,send data to group, but the user in group is empty\n");
		return;
	}

	struct user_info *usr = ur->user_data;

	x_printf(D, "mfptp_find_usr_cb---------thread=%lu\n", cur);
	LOG(LOG_NET_DOWNLINK, D, "mfptp_find_usr_cb---------thread=%lu\n", cur);

	MFPTP_WORKER_PTHREAD *p_worker = usr->processor;
	cache_add(&(usr->send), p_worker->task.data, p_worker->task.data_size);
	ev_io *p_watcher = &(usr->o_watcher);
	p_watcher->data = usr;
	int active = ev_is_active(p_watcher);

	if (!active) {
		ev_io_init(p_watcher, _mfptp_send_cb, usr->sfd, EV_WRITE);
		ev_io_start(p_worker->evuv.loop, p_watcher);
	}
}

/**********************************SERVER******************************************/
static void mfptp_task_handle(void *data, void *addr)
{
	struct user_info        *usr;
	int                     b_is_added = 0;
	MFPTP_WORKER_PTHREAD    *p_worker = data;
	ev_io                   *p_watcher;

	while (free_queue_pull(&(p_worker->tlist), (void *)&(p_worker->task))) {
		if (enMODE_USR == p_worker->task.mode) {
			// 修改  添加条件usr->auth_status判断用户是否在线
			usr = rb_search_user(&user_tree, p_worker->task.usrID);

			if (NULL == usr) {
				x_printf(E, "出错啦，消息接收用户不存在，你让我肿么办哪，丢掉还是暂存哪\n");
				LOG(LOG_NET_DOWNLINK, E, "error, the user receiving data is not exist ,please tell me how to deal with the data.\n");
				free(p_worker->task.data);			// 防止内存泄漏
				return;
			} else {
				if (usr->online == MFPTP_ONLINE) {
					x_printf(E, "消息接收用户在线，我会努力的发送给他的\n");
					LOG(LOG_NET_DOWNLINK, E, "the user is online ,we will send data to it\n");
				} else {
					x_printf(E, "出错啦，消息接收用户没在线，你让我肿么办哪，丢掉还是暂存哪\n");
					LOG(LOG_NET_DOWNLINK, E, "error, the user receiving data is not online ,please tell me how to deal with the data.\n");
					free(p_worker->task.data);			// 防止内存泄漏
					return;
				}
			}

			cache_add(&(usr->send), p_worker->task.data, p_worker->task.data_size);
			free(p_worker->task.data);				// 防止内存泄漏
			p_watcher = &(usr->o_watcher);
			p_watcher->data = usr;
			int active = ev_is_active(p_watcher);

			if (!active) {
				ev_io_init(p_watcher, _mfptp_send_cb, usr->sfd, EV_WRITE);
				ev_io_start(p_worker->evuv.loop, p_watcher);
			}
		} else if (enMODE_GRP == p_worker->task.mode) {
			usr = rb_search_user(&user_tree, "100000000000000");
			topo_select_online_user_by_channel(p_worker->task.usrID, mfptp_find_usr_cb);
			USER_RECORD ur;
			ur.user_data = usr;
		} else {}
	}
}

static void shell_cntl(const char *data)
{}

static void init_worker(MFPTP_WORKER_PTHREAD *p_worker)
{
	/*init queue list*/
	cq_init(&(p_worker->qlist));
	/*init task list*/
	free_queue_init(&(p_worker->tlist), sizeof(struct mfptp_task_node), MAX_MFPTP_QUEUE_NUMBER);

	/*set loop*/
	p_worker->loop = ev_loop_new(EVBACKEND_EPOLL | EVFLAG_NOENV);
#ifdef USE_PIPE
	/*create pipe*/
	if (pipe2(p_worker->pfds, (O_NONBLOCK | O_CLOEXEC)) < 0) {
		x_perror("pipe2");
		LOG(LOG_INIT, E, "pipe2 error\n");
		exit(EXIT_FAILURE);
	}

	if (pipe2(p_worker->weibo_pfds, (O_NONBLOCK | O_CLOEXEC)) < 0) {
		x_perror("pipe2");
		LOG(LOG_INIT, E, "pipe2 error\n");
		exit(EXIT_FAILURE);
	}

  #ifdef OPEN_OPTIMIZE
	fcntl(p_worker->pfds[0], F_SETPIPE_SZ, PIPE_MAX_SIZE);
	int size = fcntl(p_worker->pfds[0], F_GETPIPE_SZ, NULL);
	assert(size == PIPE_MAX_SIZE);
  #endif
	/*set io_watcher*/
	p_worker->pipe_watcher.data = p_worker;
	ev_io_init(&(p_worker->pipe_watcher), mfptp_fetch_cb, p_worker->pfds[0], EV_READ);
	ev_io_start(p_worker->loop, &(p_worker->pipe_watcher));

	p_worker->weibo_pipe_watcher.data = p_worker;
	ev_io_init(&(p_worker->weibo_pipe_watcher), mfptp_weibo_fetch_cb,
		p_worker->weibo_pfds[0], EV_READ);
	ev_io_start(p_worker->loop, &(p_worker->weibo_pipe_watcher));
#else
	/* init async watcher */
	p_worker->async_watcher.data = p_worker;
	ev_async_init(&(p_worker->async_watcher), mfptp_async_cb);
	ev_async_start(p_worker->loop, &(p_worker->async_watcher));	/* Listen for notifications from other threads */
#endif	/* ifdef USE_PIPE */
}

static void *mfptp_start_worker(void *arg)
{
	/* Any per-thread setup can happen here; thread_init() will block until
	 * all threads have finished initializing.
	 */
	struct safe_init_step   *info = arg;
	MFPTP_WORKER_PTHREAD    *p_worker = NULL;

	SAFE_PTHREAD_INIT_COME(info);

	int idx = info->step;

	p_worker = &g_worker_pthread[idx];

	p_worker->index = idx;
	p_worker->thread_id = pthread_self();

	init_worker(p_worker);

	if (g_mfptp_settings.conf->pthrd_init) {
		g_mfptp_settings.conf->pthrd_init(p_worker);
	}

	SAFE_PTHREAD_INIT_OVER(info);

	/* 生成zmq 的push socket， 线程通过此socket，把消息转发出去*/
	p_worker->zmq_socket = mfptp_init_push_socket(MFPTP_PUSH_SOCKET_ENDPOINT);

	if (NULL == p_worker->zmq_socket) {
		x_printf(E, "zmq socket 为空\n");
		LOG(LOG_INIT, E, "zmq socket 为空\n");
		/* 致命错误 */
	}

	p_worker->evuv.task = NULL;
	p_worker->evuv.loop = p_worker->loop;
	p_worker->evuv.tsz = sizeof(struct mfptp_task_node);
	p_worker->evuv.data = p_worker;
	p_worker->evuv.task_lookup = g_mfptp_settings.conf->task_lookup;
	p_worker->evuv.task_handle = mfptp_task_handle;

	supex_evuv_loop(&p_worker->evuv);
	return NULL;
}

int mfptp_mount(struct mfptp_cfg_list *conf)
{
	SAFE_ONCE_INIT_COME(&g_mfptp_mount_mark);

	cfg_check(conf);
	g_mfptp_settings.conf = conf;

	SAFE_ONCE_INIT_OVER(&g_mfptp_mount_mark);
	return 0;
}

int mfptp_start(void)
{
	pthread_t       proxy_id;
	pthread_t       log_id;

	broker_para broker;

	broker.front_addr = MFPTP_PUSH_SOCKET_ENDPOINT;
	broker.back_addr = MFPTP_PUBLISH_ENDPOINT;

	LOG(LOG_INIT, M, "mfptp init module start!\n");
	/*init data*/
	first_init();

	/*init proj*/
	if (g_mfptp_settings.conf->entry_init) {
		g_mfptp_settings.conf->entry_init();
	}

	g_worker_pthread = calloc(G_WORKER_COUNTS, sizeof(MFPTP_WORKER_PTHREAD));
	assert(g_worker_pthread);
	pthread_create(&proxy_id, NULL, start_proxy, &broker);
	/*创建日志线程，为了响应信号*/
	pthread_create(&log_id, NULL, start_change_log, NULL);

	/* 启动worker线程,上行下行数据的收发 */
	safe_start_pthread((void *)mfptp_start_worker, G_WORKER_COUNTS, NULL, NULL);

	g_mfptp_init_done = true;

	/* 初始化master线程的监听连接socket */
	int listenfd = socket_init(g_mfptp_settings.conf->file_info.srv_port);

	g_master_pthread.robin = 0;

	/* 记录master线程的thread_id */
	g_master_pthread.thread_id = pthread_self();

	/* 主线程使用默认的loop循环 */
	g_master_pthread.loop = ev_default_loop(0);

	/*register signal*/
	ev_signal_init(&(g_master_pthread.signal_watcher), mfptp_signal_cb, SIGQUIT);	// TODO
	ev_signal_start(g_master_pthread.loop, &(g_master_pthread.signal_watcher));

	/*kill -10 pid 可以重新加载日志级别*/
	ev_signal_init(&(g_master_pthread.log_signal_watcher), mfptp_signal_cb, SIGUSR1);	// TODO
	ev_signal_start(g_master_pthread.loop, &(g_master_pthread.log_signal_watcher));

	/*set msmq*/
	bool ok = msmq_init(g_mfptp_settings.conf->argv_info.msmq_name, shell_cntl);

	if (ok) {
		ev_io_init(&(g_master_pthread.msmq_watcher), msmq_share_cb, msmq_hand(), EV_READ);
		ev_io_start(g_master_pthread.loop, &(g_master_pthread.msmq_watcher));
	}

	/*set update timer*/
	ev_timer_init(&(g_master_pthread.update_watcher), mfptp_update_cb, get_overplus_time(), 0.);
	ev_timer_start(g_master_pthread.loop, &(g_master_pthread.update_watcher));

	/* 设置监听连接到来的回调函数， 所有与服务器的连接到在mfptp_accept_cb中处理 */
	ev_io_init(&(g_master_pthread.accept_watcher), mfptp_accept_cb, listenfd, EV_READ);
	ev_io_start(g_master_pthread.loop, &(g_master_pthread.accept_watcher));

	LOG(LOG_INIT, M, "mfptp init module OK!\n\n\n");
	ev_loop(g_master_pthread.loop, 0);
	ev_loop_destroy(g_master_pthread.loop);

	if (ok) {
		msmq_exit();
	}

	return 0;
}

