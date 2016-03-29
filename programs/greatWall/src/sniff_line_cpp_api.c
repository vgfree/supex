#include <assert.h>
#include <unistd.h>

#include "sniff_api.h"
#include "sniff_line_cpp_api.h"
#include "dams_cfg.h"
#include "tcp_api.h"
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

#define RESEND_PROTECT_TIME_DELAY 2
static int safe_fresh_http(const char *host, int port, const char *data, size_t size, int mark)
{
	static __thread time_t status[MAX_LINK_INDEX] = { 0 };

	int     ok = 0;
	time_t  now = 0;
	time_t  old = status[mark];

	if (old == 0) {
		ok = sync_tcp_ask(host, port, data, size, NULL, 0, -1);	// 0 设置默认超时， -1 不设置超时
	} else {
		now = time(NULL);

		if (RESEND_PROTECT_TIME_DELAY > now - old) {
			return -1;
		} else {
			ATOMIC_CLEAR(&status[mark]);
			ok = sync_tcp_ask(host, port, data, size, NULL, 0, -1);
		}
	}

	if (ok < 0) {
		now = time(NULL);
		ATOMIC_SET(&status[mark], now);
		x_printf(W, "links[%d] is forbid to forward data for %d second!", mark, RESEND_PROTECT_TIME_DELAY);
	}

	return (ok >= 0) ? 0 : -1;
}

static int safe_delay_http(const char *host, int port, const char *data, size_t size, int mark)
{
	int             ok = 0;
	unsigned int    idx = 0;
	char            *res = NULL;

	do {
		ok = sync_tcp_ask(host, port, data, size, &res, 0, -1);	// 0 设置默认超时， -1 不设置超时

		if ((ok > 0) && res) {
			free(res);
			res = NULL;
		} else {
			if (ok != TCP_ERR_MEMORY) {
				if (0 == ++idx % 3) {
					usleep(MIN(5000 * idx, 5000000));
				}
			}
		}
	} while (ok == TCP_ERR_CONNECT || ok == TCP_ERR_SEND);
	return 0;
}

int sniff_vms_call_rpushx(void *user, void *task)
{
	struct sniff_task_node  *p_task = task;
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	time_t                  delay = time(NULL) - p_task->stamp;

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
						safe_fresh_http(host, port, data, size, i);
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
					safe_delay_http(host, port, data, size, i);
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
						safe_fresh_http(host, port, data, size, i);
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
						safe_delay_http(host, port, data, size, i);
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

