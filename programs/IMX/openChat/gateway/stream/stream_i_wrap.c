#include "iniparser.h"
#include "libmini.h"
#include "manage_skts.h"

#include "stream_i_wrap.h"

#define CONFIG                          "streamGateway.conf"
#define BIND_EXCHAGE_STREAM_HOST        ":BindExchageStreamHost"
#define BIND_EXCHAGE_STREAM_PORT        ":BindExchageStreamPort"

static struct comm_context *g_commctx = NULL;
struct manage_skts g_stream_i_skts = { .used = 0, .rwlock = PTHREAD_RWLOCK_INITIALIZER };

static void core_exchange_node_cb(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;

	x_printf(D, "callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			printf("streamGateway here is accept : %d\n", socket);
			manage_skts_add(&g_stream_i_skts, socket);
			break;

		case STEP_ERRO:
			printf("streamGateway here is error : %d\n", socket);
			commapi_close(commctx, socket);
			break;

		case STEP_WAIT:
			printf("streamGateway here is wait : %d\n", socket);
			break;

		case STEP_STOP:
			printf("streamGateway here is close : %d\n", socket);
			manage_skts_del(&g_stream_i_skts, socket);
			break;

		default:
			printf("unknow!\n");
			break;
	}
}

int stream_i_wrap_init(void)
{
	dictionary      *config = iniparser_load(CONFIG);
	const char            *host = iniparser_getstring(config, BIND_EXCHAGE_STREAM_HOST, NULL);
	const char            *port = iniparser_getstring(config, BIND_EXCHAGE_STREAM_PORT, NULL);

	x_printf(D, "nodeServer:%s, nodePort:%s.", host, port);

	g_commctx = commapi_ctx_create();

	if (!g_commctx) {
		return -1;
	}

	struct comm_cbinfo callback_info = {};
	callback_info.fcb = core_exchange_node_cb;
	assert(commapi_socket(g_commctx, host, port, &callback_info, COMM_BIND) != -1);

	iniparser_freedict(config);
	return 0;
}

void stream_i_wrap_exit(void)
{
	commapi_ctx_destroy(g_commctx);
}

int stream_i_wrap_recv(struct comm_message *msg)
{
	assert(msg);
	return commapi_recv(g_commctx, msg);
}

int stream_i_wrap_send(struct comm_message *msg)
{
	assert(msg);
	return commapi_send(g_commctx, msg);
}

