#include "iniparser.h"
#include "libmini.h"
#include "../manage_skts.h"

#include "status_o_wrap.h"

#define CONFIG                  "statusGateway.conf"
#define BIND_APPSRV_STATUS_HOST ":BindAppsrvStatusHost"
#define BIND_APPSRV_STATUS_PORT ":BindAppsrvStatusPort"
#define CONN_USRAPI_STATUS_HOST ":ConnUsrapiStatusHost"
#define CONN_USRAPI_STATUS_PORT ":ConnUsrapiStatusPort"

static struct comm_context *g_commctx = NULL;
struct manage_skts g_status_o_skts = { .used = 0, .rwlock = PTHREAD_RWLOCK_INITIALIZER };

static void status_gateway_cb(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;

	x_printf(D, "callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			printf("server here is accept : %d\n", socket);
			manage_skts_add(&g_status_o_skts, socket);
			break;

		case STEP_ERRO:
			printf("server here is error : %d\n", socket);
			commapi_close(commctx, socket);
			break;

		case STEP_WAIT:
			printf("server here is wait : %d\n", socket);
			break;

		case STEP_STOP:
			printf("server here is close : %d\n", socket);
			manage_skts_del(&g_status_o_skts, socket);
			break;

		default:
			printf("unknow!\n");
			break;
	}
}

static void notify_usrinfoapi_cb(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;

	x_printf(D, "commapi_socket callback, fd:%d, status:%d.", socket, step);
	switch (step)
	{
		case STEP_INIT:
			// connected.
			printf("client here is connect : %d\n", socket);
			break;

		case STEP_ERRO:
			printf("client here is error : %d\n", socket);
			break;

		case STEP_WAIT:
			printf("client here is wait : %d\n", socket);
			break;

		case STEP_STOP:
			// closed.
			printf("client here is close : %d\n", socket);
			break;

		default:
			printf("unknow!\n");
			break;
	}
}

static int g_api_sfd = 0;
int status_o_get_api_sfd(void)
{
	return g_api_sfd;
}

int status_o_wrap_init(void)
{
	g_commctx = commapi_ctx_create();

	if (!g_commctx) {
		return -1;
	}

	dictionary      *config = iniparser_load(CONFIG);
	const char            *host = iniparser_getstring(config, BIND_APPSRV_STATUS_HOST, NULL);
	const char            *port = iniparser_getstring(config, BIND_APPSRV_STATUS_PORT, NULL);

	struct comm_cbinfo cbinfo = {};
	cbinfo.monitor = true;
	cbinfo.fcb = status_gateway_cb;
	assert(commapi_socket(g_commctx, host, port, &cbinfo, COMM_BIND) != -1);

	host = iniparser_getstring(config, CONN_USRAPI_STATUS_HOST, NULL);
	port = iniparser_getstring(config, CONN_USRAPI_STATUS_PORT, NULL);

	cbinfo.monitor = true;
	cbinfo.fcb = notify_usrinfoapi_cb;
	g_api_sfd = commapi_socket(g_commctx, host, port, &cbinfo, COMM_CONNECT);

	if (g_api_sfd <= 0) {
		x_printf(E, "connect usrinfoapi failed.");
		return -1;
	}

	iniparser_freedict(config);
	return 0;
}

void status_o_wrap_exit(void)
{
	commapi_ctx_destroy(g_commctx);
}

int status_o_wrap_recv(struct comm_message *msg)
{
	assert(msg);
	return commapi_recv(g_commctx, msg);
}

int status_o_wrap_send(struct comm_message *msg)
{
	assert(msg);
	return commapi_send(g_commctx, msg);
}

