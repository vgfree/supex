#include <assert.h>
#include <unistd.h>

#include "mttp_common.h"
#include "sniff_api.h"
#include "sniff_evuv_cpp_api.h"
#include "rr_cfg.h"
#include "tcp_api.h"
#include "async_api.h"
#include "apply_def.h"
#include "pool_api.h"
#include "utils.h"
#include "rr_cfg.h"
#include "parser_mttp.h"

#define MTP_VERSION     0x10
#define MTP_FORMAT      0x20

#define REDIS_BUFF_SIZE 1 << 11

extern kv_handler_t     *count_handler;
int                     sendToother(void *sfd, int size, char *result);

#define REDIS_ERR       -1
#define REDIS_OK        0
extern struct rr_cfg_file g_rr_cfg_file;

static void _vms_erro(void **base)
{}

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

int import_to_redis(char command[], void *loop, char host[], unsigned short port)
{
	x_printf(D, "import_to_redis:%s\n", command);
	struct cnt_pool         *cpool = NULL;
	struct async_ctx        *ac = NULL;

	ac = async_initial(loop, QUEUE_TYPE_FIFO, NULL, NULL, NULL, 2);

	if (ac) {
		void    *sfd = (void *)-1;
		int     rc = conn_xpool_gain(&cpool, host, port, &sfd);

		if (rc) {
			//                        conn_xpool_free ( cpool, &sfd );
			async_distory(ac);
			return -1;
		}

		/*data*/
		char    *proto;
		int     ok = cmd_to_proto(&proto, command);

		if (ok == REDIS_ERR) {
			conn_xpool_push(cpool, &sfd);
			async_distory(ac);
			return -1;
		}

		/*send*/
		async_command(ac, PROTO_TYPE_REDIS, (int)sfd, cpool, NULL, NULL, proto, strlen(proto));
		free(proto);

		async_startup(ac);
		return 0;
	}

	return -1;
}

int sniff_vms_call(void *user, void *task)
{
	struct sniff_task_node  *p_task = task;
	time_t                  delay = time(NULL) - p_task->stamp;

	x_printf(I, "task <shift> %d\t<come> %ld\t<delay> %ld",
		p_task->base.shift, p_task->stamp, delay);

	x_printf(D, "%s\n", p_task->data);
	struct supex_evcoro     *p_evcoro = supex_get_default();
	struct ev_loop          *loop = p_evcoro->scheduler->listener;

	// TODO
	x_printf(D, "get data-------------------\n");

	data_count_t    dt = { 0, 0, 0, { 0 } };
	gps_data_t      gps_dt;
	int             ret = parse_body(p_task->data, p_task->size, &dt, &gps_dt);

	if (ret == GV_ERR) {
		x_printf(E, "parse mttp body failed\n");
		return GV_ERR;
	}

	int mem = city_data_count(count_handler, &dt);

	if (mem < 0) {
		x_printf(E, "count failed .\n");
	}

	char    buff[REDIS_BUFF_SIZE];
	int     i;
	time_t  now;	// FIXME: should use collectTime as the key time
	time(&now);
	struct tm timenow = { 0 };
	localtime_r(&now, &timenow);

	for (i = 0; i < gps_dt.size; i++) {
		memset(buff, 0, sizeof(buff));
		snprintf(buff, REDIS_BUFF_SIZE, "SADD %d%02d%02d%02d%d:activeUser %s",
			timenow.tm_year + 1900, timenow.tm_mon + 1, timenow.tm_mday,
			timenow.tm_hour, (int)(timenow.tm_min / 10), gps_dt.data[i].imei);

		if (import_to_redis(buff, loop, g_rr_cfg_file.tsdb_host, g_rr_cfg_file.tsdb_port) < 0) {
			x_printf(E, "[** FAILED **] Failed to save to redis by execute:[%s]\n", buff);
		}

		memset(buff, 0, sizeof(buff));
		snprintf(buff, REDIS_BUFF_SIZE, "SADD GPS:%s:%d%02d%02d%02d%d %u|%u|%u|%d|%d",
			gps_dt.data[i].imei, timenow.tm_year + 1900, timenow.tm_mon + 1, timenow.tm_mday,
			timenow.tm_hour, (int)(timenow.tm_min / 10), gps_dt.data[i].gpstime,
			gps_dt.data[i].lon, gps_dt.data[i].lat, gps_dt.data[i].speed, gps_dt.data[i].direction);

		if (import_to_redis(buff, loop, g_rr_cfg_file.tsdb_host, g_rr_cfg_file.tsdb_port) < 0) {
			x_printf(E, "[** FAILED **] Failed to save to redis by execute:[%s]\n", buff);
		}
	}

	/*char            *result = NULL;
	 *   data_count_t    dt = { 0, 0, 0, 0 };
	 *   int             size = cjson_topb(p_task->data, &result, &dt);
	 *
	 *   if (size == GV_FILTER) {// lon and lat is not in valid city
	 *        return -1;
	 *   }
	 *
	 *   if (!result || (size < 0)) {
	 *        x_printf(E, "cjson to protocol buffer failed !\n");
	 *        return GV_ERR;
	 *   }
	 *
	 *   struct cnt_pool *cpool = NULL;
	 *   void            *sfd = (void *)-1;
	 *   int             rc = conn_xpool_gain(&cpool, g_rr_cfg_file.gaode_host, g_rr_cfg_file.gaode_port, &sfd);
	 *
	 *   if (rc) {
	 *        //		conn_xpool_free ( cpool, &sfd );
	 *        free(result);
	 *        return GV_ERR;
	 *   }
	 *
	 *   if (sendToother(sfd, size, result) < 0) {
	 *        conn_xpool_free(cpool, &sfd);
	 *        free(result);
	 *        return GV_ERR;
	 *   }
	 *
	 *   int mem = city_data_count(count_handler, &dt);
	 *
	 *   if (mem < 0) {
	 *        printf("count failed .\n");
	 *   }
	 *
	 *   conn_xpool_push(cpool, &sfd);
	 *   free(result);*/
	return GV_OK;
}

int sendToother(void *sfd, int size, char *result)
{
	unsigned int net_size = htonl((unsigned int)size);

	int     len = 0;
	char    headbuf[6];

	headbuf[0] = MTP_VERSION;
	headbuf[1] = MTP_FORMAT;
	memcpy(headbuf + 2, (char *)&net_size, 4);
#ifndef _GAODE
	len = 6;
#else
	len = 4;
#endif
	int out = 0;

	while (out < len) {
		int now = send(sfd, headbuf + out + 6 - len, len - out, 0);	// FIXME sfd to int
		x_printf(I, "send datahead len:%d\n", now);

		if (now >= 0) {
			out += now;
		} else {
			x_printf(E, "send head error:%d-->%s.\n", errno, strerror(errno));
			return GV_ERR;
		}
	}

	x_printf(I, "send headsize:%d\n", out);
	out = 0;

	while (out < size) {
		int now = send(sfd, result + out, size - out, 0);
		x_printf(I, "send size:%d\n", now);

		if (now >= 0) {
			out += now;
		} else {
			x_printf(E, "send error:%d-->%s!\n", errno, strerror(errno));
			return GV_ERR;
		}
	}

	x_printf(I, "send bodysize:%d--size is %d\n", out, size);
	return GV_OK;
}

