#include <assert.h>
#include <unistd.h>

#include "minor/sniff_api.h"
#include "sniff_evcoro_cpp_api.h"
#include "rr_cfg.h"
#include "async_tasks/async_api.h"
#include "pool_api/conn_xpool_api.h"
#include "base/utils.h"
#include "redis_api/redis_status.h"
#include "redis_parse.h"

#include "send_data.h"
#define REDIS_ERR       -1
#define REDIS_OK        0
extern struct rr_cfg_file       g_rr_cfg_file;
extern void                     *g_zmq_socket;

int import_to_redis(char command[], void *loop, char host[], unsigned short port)
{

	struct xpool    *cpool = conn_xpool_find(host, port);
	struct async_api *api = async_api_initial(loop, 1, true, QUEUE_TYPE_FIFO, NEXUS_TYPE_TEAM, NULL, NULL, NULL);
	// x_printf(D,"import_to_redis: %s\n",command);

	if (api && cpool) {
		/*data*/
		char    *proto;
		int     ok = cmd_to_proto(&proto, command);

		if (ok == REDIS_ERR) {
			async_api_distory(api);
			return -1;
		}

		/*send*/
		struct command_node *cmd = async_api_command(api, PROTO_TYPE_REDIS, cpool, proto, strlen(proto), NULL, NULL);
		free(proto);
		if (cmd == NULL) {
			async_api_distory(api);
			return -1;
		}


		async_api_startup(api);
		return 0;
	}

	return -1;
}

int sniff_vms_call(void *user, union virtual_system **VMS, struct sniff_task_node *task)
{
	struct sniff_task_node  *p_task = (struct sniff_task_node *)task;
	time_t                  delay = time(NULL) - p_task->stamp;

	x_printf(D, "task <shift> %d\t<come> %ld\t<delay> %ld",
		p_task->base.shift, p_task->stamp, delay);

	x_printf(D, "%s", p_task->data);

	//  Add by liubaoan@mirrtalk.com @2015/11/23
	return pushData(p_task->data, p_task->size);

	//  End.

	// if (delay >= OVERLOOK_DELAY_LIMIT){
	//  x_printf(W, "overlook one task");
	//  return -1;
	// }

	/*
	 *    struct cnt_pool *cpool = NULL;
	 *        void *sfd = (void*)-1;
	 *        int rc = conn_xpool_gain(&cpool, g_rr_cfg_file.map_server_host, g_rr_cfg_file.map_server_port, &sfd);
	 *    if (rc){
	 *        return -1;
	 *    }
	 *
	 *        if (sendToother(sfd, p_task->size, p_task->data) < 0) {
	 *                conn_xpool_free ( cpool, &sfd );
	 *                return -1;
	 *        }
	 *
	 *    conn_xpool_push ( cpool, &sfd );
	 */
	return 0;
}

int sendToother(void *sfd, int size, char *result)
{
	// unsigned int net_size = htonl((unsigned int)size);
	int len = 0;

	len = 4;
	int out = 0;

	/*
	 *        while(out < len){
	 *                int now = send(sfd, (char *)&net_size + out, len - out, 0);
	 *                x_printf(I, "send datahead len:%d\n", now);
	 *                if (now >= 0){
	 *                        out += now;
	 *                }else{
	 *            x_printf(E, "send head error:%d-->%s.\n", errno, strerror(errno));
	 *                        return -1;
	 *                }
	 *        }
	 *    x_printf(I, "send headsize:%d\n", out);
	 *        out = 0;
	 *        while(out < size){
	 *                int now = send(sfd, result + out, size - out, 0);
	 *                x_printf(I, "send size:%d\n", now);
	 *                if (now >= 0){
	 *                        out += now;
	 *                }else{
	 *            x_printf(E, "send error:%d-->%s!\n", errno, strerror(errno));
	 *                        return -1;
	 *                }
	 *        }
	 */
	x_printf(D, "send bodysize:%d--size is %d\n", out, size);

	return 0;
}

