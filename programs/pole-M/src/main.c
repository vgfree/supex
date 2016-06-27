#include <assert.h>
#include <zmq.h>

#include "evt_worker.h"
#include "evt_hander.h"

#include "storage_impl.h"
#include "libmini.h"
#include "conf.h"

#include "base/hashmap.h"
#include "xmq.h"
#include "netmod.h"
#include "libmini.h"

#include "pole_common.h"
#include "major/swift_api.h"
#include "load_swift_cfg.h"
#include "swift_cpp_api.h"

// ---------------Global Area---------------------//
//
struct swift_cfg_list   g_swift_cfg_list = {};
struct pole_conf        g_pole_conf = {};

// g_xmq_p1: Just for swift module.
xmq_producer_t *g_xmq_p1 = NULL;

// ---------------Global Area End-----------------//

void *data_write_swift(void *args)
{
	xmq_ctx_t *x_ctx = (xmq_ctx_t *)args;
	
	xmq_register_producer(x_ctx, "P1");
	g_xmq_p1 = xmq_get_producer(x_ctx, "P1");
	assert(g_xmq_p1 != NULL);

	switch (g_swift_cfg_list.file_info.ptype)
	{
		case USE_HTTP_PROTO:
		{
			g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
			g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_call;

			break;
		}

		case USE_REDIS_PROTO:
		{
			g_swift_cfg_list.func_info[LPUSHX_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
			g_swift_cfg_list.func_info[LPUSHX_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_call;
			g_swift_cfg_list.func_info[RPUSHX_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
			g_swift_cfg_list.func_info[RPUSHX_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_call;

			break;
		}

		case USE_MTTP_PROTO:	// fixme
		{
			g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
			g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_call;

			break;
		}
#if 0
		case USE_MFPTP_PROTO:	// MFPTP Protocal - doesn't implement yet!
			break;
#endif
	}

	swift_mount(&g_swift_cfg_list);
	swift_start();

	return NULL;
}

void *data_write_zmq(void *args)
{
	xmq_ctx_t *x_ctx = (xmq_ctx_t *)args;

	xmq_register_producer(x_ctx, "P2");
	xmq_producer_t *producer = xmq_get_producer(x_ctx, "P2");
	assert(producer != NULL);

	void    *z_ctx = zmq_ctx_new();
	void    *z_skt = zmq_socket(z_ctx, ZMQ_REP);

	int res = zmq_bind(z_skt, g_pole_conf.input_uri);
	assert(res == 0);

	const char *resp = "OK";

	while (1) {
		zmq_msg_t z_msg;

		/*recv*/
		res = zmq_msg_init(&z_msg);

		if (res != 0) {
			x_printf(F, "zmq_msg_init: fail. Error-%s.", zmq_strerror(errno));
			break;
		}

		res = zmq_msg_recv(&z_msg, z_skt, 0);

		if (res == -1) {
			x_printf(E, "zmq_msg_recv: fail. Error-%s.", zmq_strerror(errno));
			break;
		}

		/*work*/
		xmq_msg_t *x_msg = xmq_msg_new_data(zmq_msg_data(&z_msg), zmq_msg_size(&z_msg));
		zmq_msg_close(&z_msg);

		if (x_msg == NULL) {
			x_printf(E, "xmq_msg_new_data: fail. Error-%s.", strerror(errno));
			break;
		}

		res = xmq_push_tail(producer, x_msg);

		if (res != 0) {
			x_printf(E, "xmq_push_tail: fail. Error-%s.", strerror(errno));
		}

		xmq_msg_destroy(x_msg);

		/*send*/
		res = zmq_msg_init_data(&z_msg, (void *)resp, strlen(resp) + 1, NULL, NULL);

		if (res != 0) {
			x_printf(E, "zmq_msg_init_data: fail. Error-%s.", zmq_strerror(errno));
			break;
		}

		res = zmq_msg_send(&z_msg, z_skt, 0);
		zmq_msg_close(&z_msg);

		if (res != 3) {
			x_printf(E, "zmq_msg_send: fail. Error-%s.", zmq_strerror(errno));
			break;
		}
	}

	zmq_close(z_skt);
	zmq_ctx_destroy(z_ctx);

	x_printf(F, "Thread-> For ZeroMQ protol has already exit!");

	return NULL;
}

static int input_start(xmq_ctx_t *ctx)
{
	pthread_t thrd_swift, thrd_zmq;

	int res = pthread_create(&thrd_swift, NULL, data_write_swift, ctx);
	assert(res == 0);
	x_printf(I, "Thread->For Swift Protocol startup succeed. ID:%ld", thrd_swift);


	res = pthread_create(&thrd_zmq, NULL, data_write_zmq, ctx);
	assert(res == 0);
	x_printf(I, "Thread->For ZeroMQ Protocol startup succeed. ID:%ld", thrd_zmq);
}

int main(int argc, char **argv)
{
	// Loading Swift's configuration and startup.
	load_supex_args(&g_swift_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);
	// Loading the SWIFT's configuation.
	load_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	// Loading Pole-M's configuration.
	config_init(&g_pole_conf, g_swift_cfg_list.argv_info.conf_name);

	// Init logs
	char path[MAX_PATH_SIZE] = {};
	snprintf(path, sizeof(path), "%s/%s.log", g_swift_cfg_list.file_info.log_path, g_swift_cfg_list.file_info.log_file);
	SLogOpen(path, SLogIntegerToLevel(g_swift_cfg_list.file_info.log_level));

	// Init XMQ's context.
	xmq_ctx_t *xmq_ctx = xmq_context_init(POLE_DATA_PATH, g_pole_conf.max_records,
			ldb_pvt_create, driver_ldb_put, driver_ldb_get, ldb_pvt_destroy);
	assert(xmq_ctx != NULL);

	// Init NETMOD's context.
	evt_ctx_t *evt_ctx = evt_ctx_init(SOCK_SERVER, g_pole_conf.bind_uri, NULL);
	assert(evt_ctx);
	// Startup the thread of Network Center to Recv & Send data.
	pthread_t       thrd;
	pthread_attr_t  attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int ok = pthread_create(&thrd, &attr, work_evt, evt_ctx);
	assert(ok == 0);

	// Startup the input data thread.
	input_start(xmq_ctx);

	// 用于保存(KEY: consumer VALUE: 存储的每个consumer对象的指针
	hashmap_t *g_hash_table = hashmap_open();
	if (!g_hash_table) {
		x_printf(E, "Create Thread Hash fail!");
		return -1;
	}

	// Startup the output data thread.(内含协程处理)
	int i;
	int all = g_pole_conf.event_worker_counts;
	tlpool_t *tlpool = tlpool_init(all, 10000, sizeof(struct online_task), NULL);
	for (i = 0; i < all; i++) {
		tlpool_bind(tlpool, (void (*)(void *))event_handler_startup, tlpool, i);
	}
	tlpool_boot(tlpool);

	// Startup the dispatch thread.
	/* 第一次连上来的一定是主节点. */
	x_printf(W, "First request client must be 'master'. " \
			"Checking the pole-S_conf.json file's SLAVE_UNIQUE_ID");
	event_dispenser_startup(xmq_ctx, evt_ctx, tlpool, g_hash_table, all);


	hashmap_close(g_hash_table);
	pthread_kill(thrd, SIGQUIT);
	evt_ctx_destroy(evt_ctx);
	xmq_unregister_producer(xmq_ctx, "P1");
	xmq_context_destroy(xmq_ctx);

	return 0;
}

