#include <assert.h>
#include <unistd.h>

#include "minor/sniff_api.h"
#include "sniff_evcoro_cpp_api.h"
#include "dams_cfg.h"
#include "tcp_api/tcp_api.h"
#include "pools/pool2.h"
#include "pool_api/connpool2_api.h"
#include "async_tasks/async_api.h"
#include "apply_def.h"
#include "base/utils.h"
#include "spx_evcs.h"

extern struct dams_cfg_file g_dams_cfg_file;


#if 1
static void do_over_clean(struct async_obj *obj, void *reply, void *data)
{
	struct pool2         *cpool = data;
	if (obj->replies.work->done) {
		pool_api_push(cpool, obj->replies.work->sfd);
	} else {
		x_printf(E, "Disconnected...");
		pool_api_free(cpool, obj->replies.work->sfd);
	}

}

static int forward_to_server(char *host, int port, const char *data, size_t size, struct ev_loop *loop)
{
	struct pool2         *cpool = NULL;
	struct async_ctx        *ac = NULL;

	ac = async_initial(loop, QUEUE_TYPE_FIFO, NEXUS_TYPE_TEAM, NULL, NULL, NULL, 1);

	if (ac) {
		void    *sfd = (void *)(intptr_t)-1;
		int     rc = pool_api_gain(&cpool, host, port, &sfd);

		if (rc) {
			async_distory(ac);
			return (POOL_API_ERR_IS_FULL == rc) ? -2 : -1;
		}

		/*send*/
		async_command(ac, PROTO_TYPE_HTTP, (int)(intptr_t)sfd, do_over_clean, cpool, data, size);

		async_startup(ac);
		return 0;
	}

	return -1;
}

static int safe_fresh_http(const char *host, int port, const char *data, size_t size, int mark)
{
	static __thread time_t status[MAX_LINK_INDEX] = { 0 };
	struct ev_loop          *loop = supex_get_default()->scheduler->listener;

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
			AO_CLEAR(&status[mark]);
			ok = forward_to_server(host, port, data, size, loop);
		}
	}

	if (ok < 0) {
		if (ok == -2) {
			x_printf(W, "links[%d] is busy now, pool is full!", mark);
		} else {
			now = time(NULL);
			AO_SET(&status[mark], now);
			x_printf(W, "links[%d] is forbid to forward data for %d second!", mark, RESEND_PROTECT_TIME_DELAY);
		}
	}

	return (ok >= 0) ? 0 : -1;
}

static int safe_delay_http(const char *host, int port, const char *data, size_t size, int mark)
{
	struct pool2         *cpool = NULL;
	struct async_ctx        *ac = NULL;
	unsigned int            idx = 0;
	int                     rc = 0;
	struct ev_loop          *loop = supex_get_default()->scheduler->listener;

	ac = async_initial(loop, QUEUE_TYPE_FIFO, NEXUS_TYPE_TEAM, NULL, NULL, NULL, 1);

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
		async_command(ac, PROTO_TYPE_HTTP, (int)(intptr_t)sfd, do_over_clean, cpool, data, size);

		async_startup(ac);
		return 0;
	}

	return -1;
}
#else
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
			AO_CLEAR(&status[mark]);
			ok = sync_tcp_ask(host, port, data, size, NULL, 0, -1);
		}
	}

	if (ok < 0) {
		now = time(NULL);
		AO_SET(&status[mark], now);
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
#endif

int sniff_vms_call_rlpushx(void *user, union virtual_system **VMS, struct sniff_task_node *task)
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

int sniff_vms_call_publish(void *user, union virtual_system **VMS, struct sniff_task_node *task)
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

