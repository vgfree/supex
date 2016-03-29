#include <assert.h>
#include <unistd.h>

#include "sniff_api.h"
#include "sniff_evuv_cpp_api.h"
#include "weibo_cfg.h"
#include "tcp_api.h"
#include "apply_entry.h"

extern struct weibo_cfg_file g_weibo_cfg_file;
static void _vms_erro(void **base)
{}

static int _vms_cntl(void **base, int last, struct sniff_task_node *task)
{
	struct msg_info *msg = task->data;

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

int sniff_vms_call(void *user, void *task)
{
	struct sniff_task_node  *p_task = task;
	SNIFF_WORKER_PTHREAD    *p_sniff_worker = (SNIFF_WORKER_PTHREAD *)user;
	time_t                  delay = time(NULL) - p_task->stamp;

	x_printf(S, "channel %d\t|task <thread> %p\t<shift> %d\t<come> %lu\t<delay> %ld",
		p_sniff_worker->genus, p_task->thread_id, p_task->base.shift, p_task->stamp, delay);

	if (strstr(p_task->data, "groupID")) {
		entry_group_weibo(p_sniff_worker->evuv.loop, p_task->data, p_task->size);
	}

	if (strstr(p_task->data, "regionCode")) {
		entry_city_weibo(p_sniff_worker->evuv.loop, p_task->data, p_task->size);
	}

	return 0;
}

int sniff_vms_idle(void *user, void *task)
{
	return 0;
}

