#include "iniparser.h"
#include "libmini.h"

#include "status_i_wrap.h"

#define CONFIG                          "statusGateway.conf"
#define BIND_EXCHAGE_STATUS_HOST        ":BindExchageStatusHost"
#define BIND_EXCHAGE_STATUS_PORT        ":BindExchageStatusPort"

static struct comm_context *g_commctx = NULL;

static void event_notify_from_exchange(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;

	x_printf(D, "commapi_socket callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			printf("statusGateway here is accept : %d\n", socket);
			break;

		case STEP_ERRO:
			printf("statusGateway here is error : %d\n", socket);
			commapi_close(commctx, socket);
			break;

		case STEP_WAIT:
			printf("statusGateway here is wait : %d\n", socket);
			break;

		case STEP_STOP:
			printf("statusGateway here is close : %d\n", socket);
			break;

		default:
			printf("unknow!\n");
			break;
	}
}

int status_i_wrap_init(void)
{
	dictionary      *config = iniparser_load(CONFIG);
	const char            *host = iniparser_getstring(config, BIND_EXCHAGE_STATUS_HOST, NULL);
	const char            *port = iniparser_getstring(config, BIND_EXCHAGE_STATUS_PORT, NULL);

	x_printf(D, "nodeServer:%s, nodePort:%s.", host, port);

	g_commctx = commapi_ctx_create();

	if (!g_commctx) {
		return -1;
	}

	struct comm_cbinfo cbinfo = {};
	cbinfo.monitor = true;
	cbinfo.fcb = event_notify_from_exchange;
	assert(commapi_socket(g_commctx, host, port, &cbinfo, COMM_BIND) != -1);

	iniparser_freedict(config);
	return 0;
}

void status_i_wrap_exit(void)
{
	commapi_ctx_destroy(g_commctx);
}

int status_i_wrap_recv(struct comm_message *msg)
{
	assert(msg);
	return commapi_recv(g_commctx, msg);
}

int status_i_wrap_send(struct comm_message *msg)
{
	assert(msg);
	return commapi_send(g_commctx, msg);
}

