#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <memory.h>
#include "adopt_tasks/adopt_task.h"

#include "load_cfg.h"
#include "major/smart_api.h"
static void session_dispatch_task(struct session_task *service)
{
	char type = 0;

	assert(service);
	assert(service->action);

	if ((service->action->taskmode != BIT8_TASK_TYPE_WHOLE) &&
		(service->action->taskmode != BIT8_TASK_TYPE_ALONE)) {
		type = BIT8_TASK_TYPE_ALONE;
	} else {
		type = service->action->taskmode;
	}

	struct adopt_task_node task = {
		.id     = 0,
		.sfd    = 0,
		.type   = type,
		.origin = BIT8_TASK_ORIGIN_MSMQ,
		.func   = (TASK_VMS_FCB)service->action->action,
		.index  = 0,
		.data   = (void *)service
	};

	if (type == BIT8_TASK_TYPE_WHOLE) {
		smart_all_task_hit(&task, false, service->fd);
	} else {
		smart_one_task_hit(&task, false, service->fd);
	}
}

struct smart_cfg_list g_smart_cfg_list = {};

static void entry_init(void)
{
	if (!kvpool_init()) {
		exit(EXIT_FAILURE);
	}

}

/*

viod *t_msg_recv(void)
{
	while(1)
	{
		struct app_msg  msg = {};
		int             more = 0;
		recv_app_msg(&msg, &more, -1);
		printf("msg size:%d, [0]iov_len:%d\n",
			   msg.vector_size, msg.vector[0].iov_len);
	}
}
*/
int smart_vms_call_set(void *user, union virtual_system **VMS, struct adopt_task_node *task)
{
	struct data_node        *p_node = get_pool_data(task->sfd);
	char                    *p_buf = cache_data_address(&p_node->mdl_recv.cache);
	printf("p_buf = %s",p_buf);
}



int main(int argc, char **argv)
{
/*	pthread tid_recv;
	int ret = 0;
	create_io();
	ret = pthread_create(&tid_send, NULL, (viod *)t_msg_recv , NULL)
	if( ret != 0)
	{
		printf("Create send pthread error!\n");
		return -1;
	}
*/
	
	load_supex_args(&g_smart_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);

	load_cfg_file(&g_smart_cfg_list.file_info, g_smart_cfg_list.argv_info.conf_name);

	g_smart_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
	g_smart_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_VMS_FCB)smart_vms_call_set;


	smart_mount(&g_smart_cfg_list);
	smart_start();
	return 0;
}

