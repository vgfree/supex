#include <assert.h>
#include <zmq.h>

#include "porter.h"
#include "switcher.h"

#include "event_dispenser.h"
#include "event_handler.h"

#include "ldb_cb.h"
#include "log.h"
#include "libmini.h"
#include "conf.h"

#include "xmq.h"
#include "netmod.h"

// #include "supex.h"
#include "major/swift_api.h"
#include "load_swift_cfg.h"
#include "swift_cpp_api.h"

// ---------------Global Area---------------------//
//
struct swift_cfg_list g_swift_cfg_list = {};

// g_xmq_producer: Just for swift module.
xmq_producer_t *g_xmq_producer = NULL;

// g_iProType: The input data protocol.
enum pole_protype g_iProType = -1;

// ---------------Global Area End-----------------//

void *data_write_swift()
{
	// g_iProType was initialized by load_cfg_file.
	switch (g_iProType & 0xFFFFF0)
	{
		case POLE_PROTYPE_HTTP:	// HTTP Protocal.
		{
			g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
			g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_call;

			break;
		}

		case POLE_PROTYPE_REDIS:// REDIS Protocal.
		{
			g_swift_cfg_list.func_info[LPUSHX_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
			g_swift_cfg_list.func_info[LPUSHX_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_call;
			g_swift_cfg_list.func_info[RPUSHX_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
			g_swift_cfg_list.func_info[RPUSHX_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_call;

			break;
		}

		case POLE_PROTYPE_MTTP:	// MTTP Protocal.
		{
			g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].type = BIT8_TASK_TYPE_ALONE;
			g_swift_cfg_list.func_info[APPLY_FUNC_ORDER].func = (TASK_VMS_FCB)swift_vms_call;

			break;
		}

		case POLE_PROTYPE_MFPTP:// MFPTP Protocal - doesn't implement yet!
			break;
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

		xmq_msg_t *x_msg = xmq_msg_new_data(zmq_msg_data(&z_msg), zmq_msg_size(&z_msg));
		zmq_msg_close(&z_msg);

		if (x_msg == NULL) {
			x_printf(E, "xmq_msg_new_data: fail. Error-%s.", strerror(errno));
			break;
		}

		res = xmq_push_tail(producer, x_msg);
		xmq_msg_destroy(x_msg);

		if (res != 0) {
			x_printf(E, "xmq_push_tail: fail. Error-%s.", strerror(errno));
			break;
		}

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

int data_write_start(xmq_ctx_t *ctx)
{
	pthread_t thrd_swift, thrd_zmq;

	int res = pthread_create(&thrd_swift, NULL, data_write_swift, NULL);

	usleep(10000);
	assert(res == 0);
	x_printf(I, "Thread->For Swift Protocol startup succeed. ID:%ld", thrd_swift);

	if (g_iProType & 0x01) {
		res = pthread_create(&thrd_zmq, NULL, data_write_zmq, ctx);
		usleep(10);
		assert(res == 0);
		x_printf(I, "Thread->For ZeroMQ Protocol startup succeed. ID:%ld", thrd_zmq);
	}
}

#ifdef TC_THREAD
int __load_porters(event_ctx_t *ev_ctx, xmq_ctx_t *xmq_ctx)
{
	xlist_t *iter;

	list_foreach(iter, &xmq_ctx->list_consumers)
	{
		xmq_consumer_t *consumer = (xmq_consumer_t *)container_of(iter, xmq_consumer_t, node);

		if (consumer) {
			porter_info_t *porter = porter_create(consumer->identity, consumer, ev_ctx, consumer->last_fetch_seq);

			if (porter) {
				pthread_mutex_lock(&porters_list_lock);
				list_add_tail(&porter->list, &porters_list_head);
				pthread_mutex_unlock(&porters_list_lock);

				if (porter_work_start(porter)) {
					x_printf(S, "Thread of work porter start fail. Consumer ID:[%s]", consumer->identity);
					return -1;
				}
			}
		}
	}
	return 0;
}
#endif

int main(int argc, char **argv)
{
	// Loading Swift's configuration and startup.
	load_supex_args(&g_swift_cfg_list.argv_info, argc, argv, NULL, NULL, NULL);
	// Loading the SWIFT's configuation.
	load_cfg_file(&g_swift_cfg_list.file_info, g_swift_cfg_list.argv_info.conf_name);

	// Loading Pole-M's configuration.
	int res = config_init("./item_conf.json");
	assert(res == 0);

	res = log_init(g_pole_conf.log_path, g_pole_conf.log_level);

	// INIT XMQ's context.
	xmq_ctx_t *xmq_ctx = xmq_context_init("./data", g_pole_conf.max_records, ldb_pvt_create, driver_ldb_put, driver_ldb_get, ldb_pvt_destroy);
	assert(xmq_ctx != NULL);

	xmq_register_producer(xmq_ctx, "P1");
	g_xmq_producer = xmq_get_producer(xmq_ctx, "P1");

	// INIT NETMOD's context.
	int error = 0;
	event_ctx_t *ev_ctx = event_ctx_init(&error, SOCK_SERVER, g_pole_conf.bind_uri, NULL);
	assert(ev_ctx);

	// Startup the input data thread.
	data_write_start(xmq_ctx);

#ifdef TC_THREAD
	x_printf(I, "MAIN: Current is thread version!");
	// Loading worker porters
	porters_list_init();
	res = __load_porters(ev_ctx, xmq_ctx);
	assert(res == 0);

	switcher_work(ev_ctx, xmq_ctx);

	// Threads were suspend, and waitting for exit.
	switcher_join_all_porter();
#elif defined(TC_COROUTINE)
	x_printf(I, "MAIN: Current is coroutine version!");
	res = event_handler_startup(ev_ctx, xmq_ctx, g_pole_conf.thread_number);
	assert(res == 0);

	res = event_dispenser_startup(ev_ctx, g_pole_conf.thread_number);
	assert(res == 0);
#endif
	event_ctx_destroy(ev_ctx);

	xmq_unregister_producer(xmq_ctx, "P1");
	xmq_context_destroy(xmq_ctx);

	return 0;
}

