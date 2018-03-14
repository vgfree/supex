#include "iniparser.h"
#include "libmini.h"

#include "appsrv_o_wrap.h"

#define CONFIG                                  "appSrv.conf"
#define CONN_GATEWAY_STATUS_HOST                ":ConnGatewayStatusHost"
#define CONN_GATEWAY_STATUS_PORT                ":ConnGatewayStatusPort"
#define CONN_GATEWAY_STREAM_HOST                ":ConnGatewayStreamHost"
#define CONN_GATEWAY_STREAM_PORT                ":ConnGatewayStreamPort"
#define CONN_USRAPI_SETTING_LOOKING_HOST        ":ConnUsrapiSettingLookingHost"
#define CONN_USRAPI_SETTING_LOOKING_PORT        ":ConnUsrapiSettingLookingPort"

static struct comm_context *g_commctx = NULL;

static void appsrv_cb(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	//struct comm_context *commctx = (struct comm_context *)ctx;

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

int     g_stream_sfd = 0;
int     g_status_sfd = 0;
int     g_usrapi_sfd = 0;

int appsrv_o_wrap_init(void)
{
	g_commctx = commapi_ctx_create();

	if (!g_commctx) {
		return -1;
	}

	struct comm_cbinfo cbinfo = {};
	cbinfo.monitor = true;
	cbinfo.fcb = appsrv_cb;

	dictionary *config = iniparser_load(CONFIG);

	const char    *host = iniparser_getstring(config, CONN_GATEWAY_STATUS_HOST, NULL);
	const char    *port = iniparser_getstring(config, CONN_GATEWAY_STATUS_PORT, NULL);
	g_status_sfd = commapi_socket(g_commctx, host, port, &cbinfo, COMM_CONNECT);

	if (g_status_sfd <= 0) {
		x_printf(E, "connect usrinfoapi failed.");
		return -1;
	}

	host = iniparser_getstring(config, CONN_GATEWAY_STREAM_HOST, NULL);
	port = iniparser_getstring(config, CONN_GATEWAY_STREAM_PORT, NULL);
	g_stream_sfd = commapi_socket(g_commctx, host, port, &cbinfo, COMM_CONNECT);

	if (g_stream_sfd <= 0) {
		x_printf(E, "connect usrinfoapi failed.");
		return -1;
	}

	host = iniparser_getstring(config, CONN_USRAPI_SETTING_LOOKING_HOST, NULL);
	port = iniparser_getstring(config, CONN_USRAPI_SETTING_LOOKING_PORT, NULL);
	g_usrapi_sfd = commapi_socket(g_commctx, host, port, &cbinfo, COMM_CONNECT);

	if (g_usrapi_sfd <= 0) {
		x_printf(E, "connect usrinfoapi failed.");
		return -1;
	}

	iniparser_freedict(config);
	return 0;
}

void appsrv_o_wrap_exit(void)
{
	commapi_ctx_destroy(g_commctx);
}

int appsrv_o_wrap_recv(struct comm_message *msg)
{
	assert(msg);
	return commapi_recv(g_commctx, msg);
}

int appsrv_o_wrap_send(struct comm_message *msg)
{
	assert(msg);
	return commapi_send(g_commctx, msg);
}

