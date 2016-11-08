#include "forward_imei.h"
#include "spx_evcs.h"
#include "evcoro_async_tasks.h"
//#include "pool_api.h"

extern struct rr_cfg_file g_rr_cfg_file;

/*检查imei是否是测试imei*/
bool forward_imei_check(char* imei)
{
	if (!imei || (g_rr_cfg_file.imei_count <= 0)) {
		return false;
	}

	int i;

	for (i = 0; i < g_rr_cfg_file.imei_count; i++) {
		//if (g_rr_cfg_file.imei_buff[i] == imei) {
		if (strncmp(g_rr_cfg_file.imei_buff[i],imei,strlen(imei) == 0)) {
			return true;
		}
	}

	return false;
}

bool forward_data(const char *data, struct ev_loop *loop, char *host, int port)
{
	if (!data) {
		return false;
	}
	char                    http_data[20480];

	char HTTP_FORMAT[] = "POST /%s HTTP/1.1\r\n"
		"User-Agent: curl/7.33.0\r\n"
		"Host: %s:%d\r\n"
		"Content-Type: application/json; charset=utf-8\r\n"
		"Connection:%s\r\n"
		"Content-Length:%d\r\n"
		"Accept: */*\r\n\r\n%s";
	snprintf(http_data, 20480 - 1, HTTP_FORMAT, "publicentry", host, port, "Keep-Alive", strlen(data), data);
        struct supex_evcoro     *p_evcoro = supex_get_default();
        struct evcoro_scheduler *p_scheduler = p_evcoro->scheduler;
        struct xpool            *cpool = conn_xpool_find(host, port);
        if(!cpool) {
                x_printf(E, "forward server err. ");
                //printf("----------------\n");
                return false;
        }

        struct async_evtasker   *tasker = evtask_initial(p_scheduler, 1, QUEUE_TYPE_FIFO, NEXUS_TYPE_SOLO);
        struct command_node     *command = evtask_command(tasker, PROTO_TYPE_HTTP, cpool, http_data, strlen(http_data));

        if(!command) {
                x_printf(E, "forward server err. ");
                //printf("----------------\n");
                evtask_distory(tasker);
                return false;
        }
        evtask_install(tasker);

        evtask_startup(p_scheduler);

        //printf("command cache buf > %s\n", command->cache.buff);

        evtask_distory(tasker);
	return true;

}
