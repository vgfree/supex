#include <assert.h>
#include <unistd.h>

#include "sniff_api.h"
#include "sniff_evuv_cpp_api.h"
#include "dams_cfg.h"
#include "tcp_api.h"
#include "pool_api.h"
#include "async_api.h"
#include "apply_def.h"
#include "utils.h"

extern struct dams_cfg_file g_dams_cfg_file;

static void _vms_erro(void **base)
{}

static int _vms_cntl(void **base, int last, struct sniff_task_node *task)
{
	struct msg_info *msg = (struct msg_info *)task->data;

	assert(msg);

	switch (msg->opt)
	{
		case 'l':
			break;

		case 'f':
			break;

		case 'o':
			break;

		case 'c':
			break;

		case 'd':
			break;

		default:
			x_printf(S, "Error msmq opt!");
			return 0;
	}
	return 0;
}

int sniff_vms_cntl(void *user, void *task)
{
	return sniff_for_alone_vm(user, task, _vms_cntl, _vms_erro);
}

static void *_vms_new(void)
{
	return NULL;
}

static int _vms_init(void **base, int last, struct sniff_task_node *task)
{
	if (*base != NULL) {
		x_printf(S, "No need to init LUA VM!");
		return 0;
	}

	*base = _vms_new();
	// assert( *base );
	return 0;
}

int sniff_vms_init(void *user, void *task)
{
	int error = 0;

	error = sniff_for_alone_vm(user, task, _vms_init, _vms_erro);

	if (error) {
		exit(EXIT_FAILURE);
	}

	return error;
}

static int _vms_exit(void **base, int last, struct sniff_task_node *task)
{
	*base = NULL;
	return 0;	/*must return 0*/
}

int sniff_vms_exit(void *user, void *task)
{
	int error = sniff_for_alone_vm(user, task, _vms_exit, _vms_erro);

	x_printf(S, "exit one alone LUA!");
	return error;
}

static int _vms_rfsh(void **base, int last, struct sniff_task_node *task)
{
	return 0;
}

int sniff_vms_rfsh(void *user, void *task)
{
	return sniff_for_alone_vm(user, task, _vms_rfsh, _vms_erro);
}

static int _vms_sync(void **base, int last, struct sniff_task_node *task)
{
	return 0;
}

int sniff_vms_sync(void *user, void *task)
{
	return sniff_for_alone_vm(user, task, _vms_sync, _vms_erro);
}

static int _vms_gain(void **base, int last, struct sniff_task_node *task)
{
	return 0;
}

int sniff_vms_gain(void *user, void *task)
{
	return sniff_for_alone_vm(user, task, _vms_gain, _vms_erro);
}

static int forward_to_server(char *host, int port, const char *data, size_t size, struct ev_loop *loop)
{
	struct cnt_pool         *cpool = NULL;
	struct async_ctx        *ac = NULL;

	ac = async_initial(loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, 1);

	if (ac) {
		void    *sfd = (void *)(intptr_t)-1;
		int     rc = pool_api_gain(&cpool, host, port, &sfd);

		if (rc) {
			async_distory(ac);
			return (POOL_API_ERR_IS_FULL == rc) ? -2 : -1;
		}

		/*send*/
		async_command(ac, PROTO_TYPE_HTTP, (int)(intptr_t)sfd, cpool, NULL, NULL, data, size);

		async_startup(ac);
		return 0;
	}

	return -1;
}

#define RESEND_PROTECT_TIME_DELAY 2
static int safe_fresh_http(char *host, int port, const char *data, size_t size, struct ev_loop *loop, int mark)
{
	static __thread time_t status[MAX_LINK_INDEX] = { 0 };

	int     ok = 0;
	time_t  now = 0;
	time_t  old = status[mark];

	if (old == 0) {
		ok = forward_to_server(host, port, data, size, loop);
	} else {
		now = time(NULL);

		if (RESEND_PROTECT_TIME_DELAY > now - old) {
			return -1;
		} else {
			ATOMIC_CLEAR(&status[mark]);
			ok = forward_to_server(host, port, data, size, loop);
		}
	}

	if (ok < 0) {
		if (ok == -2) {
			x_printf(W, "links[%d] is busy now, pool is full!", mark);
		} else {
			now = time(NULL);
			ATOMIC_SET(&status[mark], now);
			x_printf(W, "links[%d] is forbid to forward data for %d second!", mark, RESEND_PROTECT_TIME_DELAY);
		}
	}

	return (ok >= 0) ? 0 : -1;
}

static int safe_delay_http(char *host, int port, const char *data, size_t size, struct ev_loop *loop, int mark)
{
	struct cnt_pool         *cpool = NULL;
	struct async_ctx        *ac = NULL;
	unsigned int            idx = 0;
	int                     rc = 0;

	ac = async_initial(loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, 1);

	if (ac) {
		void *sfd = (void *)(intptr_t)-1;
		do {
			rc = pool_api_gain(&cpool, host, port, &sfd);

			if (rc) {
				if (0 == ++idx % 3) {
					usleep(MIN(5000 * idx, 5000000));
				}
			}
		} while (rc);

		/*send*/
		async_command(ac, PROTO_TYPE_HTTP, (int)(intptr_t)sfd, cpool, NULL, NULL, data, size);

		async_startup(ac);
		return 0;
	}

	return -1;
}

int sniff_vms_call_rpushx(void *user, void *task)
{
	struct sniff_task_node  *p_task = task;
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	time_t                  delay = time(NULL) - p_task->stamp;
	struct ev_loop          *loop = p_sniff_worker->evuv.loop;

	x_printf(S, "channel %d\t|task <shift> %d\t<come> %ld\t<delay> %ld",
		p_sniff_worker->genus, p_task->base.shift, p_task->stamp, delay);

	struct route_msg_data   *p_rmsg = (struct route_msg_data *)p_task->data;
	char                    *tags = p_rmsg->label;
	char                    *data = p_rmsg->flows;
	int                     size = p_task->size;

	int     ln = (int)MIN(strlen(tags), MAX_SNIFF_LABEL_LENGTH);
	int     lv = 0;
	char    *p = tags;
	switch (p_sniff_worker->genus)
	{
		case REAL_TIME_KIND:

			if (delay >= OVERLOOK_DELAY_LIMIT) {
				x_printf(W, "overlook one task");
				return 0;
			} else {
				do {
					lv = ln - (int)(p - tags);

					while (lv-- > 0) {
						if (*p != '|') {
							break;
						}

						p++;
					}

					if (ln <= p - tags) {
						break;
					}

					int i = x_atoi(p, ln - (int)(p - tags));
					x_printf(D, "link %d", i);

					assert(i >= 0 && i < g_dams_cfg_file.count && i < MAX_LINK_INDEX);

					if (g_dams_cfg_file.fresh[i] == IS_SET_UP) {
						char    *host = g_dams_cfg_file.links[i].host;
						int     port = g_dams_cfg_file.links[i].port;
						safe_fresh_http(host, port, data, size, loop, i);
					}

					p = strchr(p, '|');

					if (p) {
						p++;
					}
				} while (p);
				return 0;
			}

		case NON_REAL_TIME_KIND:
			do {
				lv = ln - (int)(p - tags);

				while (lv-- > 0) {
					if (*p != '|') {
						break;
					}

					p++;
				}

				if (ln <= p - tags) {
					break;
				}

				int i = x_atoi(p, ln - (int)(p - tags));
				x_printf(D, "link %d", i);

				assert(i >= 0 && i < g_dams_cfg_file.count && i < MAX_LINK_INDEX);

				if (g_dams_cfg_file.delay[i] == IS_SET_UP) {
					char    *host = g_dams_cfg_file.links[i].host;
					int     port = g_dams_cfg_file.links[i].port;
					safe_delay_http(host, port, data, size, loop, i);
				}

				p = strchr(p, '|');

				if (p) {
					p++;
				}
			} while (p);
			return 0;

		default:
			return -1;
	}
}

int sniff_vms_call_publish(void *user, void *task)
{
	struct sniff_task_node  *p_task = task;
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	time_t                  delay = time(NULL) - p_task->stamp;
	struct ev_loop          *loop = p_sniff_worker->evuv.loop;

	x_printf(S, "channel %d\t|task <shift> %d\t<come> %ld\t<delay> %ld",
		p_sniff_worker->genus, p_task->base.shift, p_task->stamp, delay);

	struct route_msg_data   *p_rmsg = (struct route_msg_data *)p_task->data;
	char                    *tags = p_rmsg->label;
	char                    *data = p_rmsg->flows;
	int                     size = p_task->size;

	switch (p_sniff_worker->genus)
	{
		case REAL_TIME_KIND:

			if (delay >= OVERLOOK_DELAY_LIMIT) {
				x_printf(W, "overlook one task");
				return 0;
			} else {
				int i;

				for (i = 0; i < g_dams_cfg_file.count; i++) {
					assert(i < MAX_LINK_INDEX);

					if (g_dams_cfg_file.fresh[i] == IS_SET_UP) {
						char    *host = g_dams_cfg_file.links[i].host;
						int     port = g_dams_cfg_file.links[i].port;
						safe_fresh_http(host, port, data, size, loop, i);
					}
				}

				return 0;
			}

		case NON_REAL_TIME_KIND:
			do {
				int i;

				for (i = 0; i < g_dams_cfg_file.count; i++) {
					assert(i < MAX_LINK_INDEX);

					if (g_dams_cfg_file.delay[i] == IS_SET_UP) {
						char    *host = g_dams_cfg_file.links[i].host;
						int     port = g_dams_cfg_file.links[i].port;
						safe_delay_http(host, port, data, size, loop, i);
					}
				}

				return 0;
			} while (0);

		default:
			return -1;
	}
}

int sniff_vms_idle(void *user, void *task)
{
	return 0;
}

