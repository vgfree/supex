#include "iniparser.h"
#include "libmini.h"

#include "usrapi_i_wrap.h"

#define CONFIG                          "usrApi.conf"
#define CONN_GATEWAY_MANAGE_HOST        ":ConnGatewayManageHost"
#define CONN_GATEWAY_MANAGE_PORT        ":ConnGatewayManagePort"

static struct comm_context *g_commctx = NULL;

static void usrapi_i_cb(void *ctx, int socket, enum STEP_CODE step, void *usr)
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

int g_manage_sfd = 0;

int usrapi_i_wrap_init(void)
{
	dictionary      *config = iniparser_load(CONFIG);
	const char            *host = iniparser_getstring(config, CONN_GATEWAY_MANAGE_HOST, NULL);
	const char            *port = iniparser_getstring(config, CONN_GATEWAY_MANAGE_PORT, NULL);

	x_printf(D, "nodeServer:%s, nodePort:%s.", host, port);

	g_commctx = commapi_ctx_create();

	if (!g_commctx) {
		return -1;
	}

	struct comm_cbinfo cbinfo = {};
	cbinfo.monitor = true;
	cbinfo.fcb = usrapi_i_cb;
	g_manage_sfd = commapi_socket(g_commctx, host, port, &cbinfo, COMM_CONNECT);

	if (g_manage_sfd <= 0) {
		x_printf(E, "connect manageGateway failed.");
		return -1;
	}

	iniparser_freedict(config);
	return 0;
}

void usrapi_i_wrap_exit(void)
{
	commapi_ctx_destroy(g_commctx);
}

int usrapi_i_wrap_recv(struct comm_message *msg)
{
	assert(msg);
	return commapi_recv(g_commctx, msg);
}

int usrapi_i_wrap_send(struct comm_message *msg)
{
	assert(msg);
	return commapi_send(g_commctx, msg);
}

