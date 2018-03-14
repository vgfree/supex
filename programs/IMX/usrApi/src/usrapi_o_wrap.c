#include "iniparser.h"
#include "libmini.h"

#include "usrapi_o_wrap.h"

#define CONFIG                                  "usrApi.conf"
#define BIND_APPSRV_SETTING_LOOKING_HOST        ":BindAppsrvSettingLookingHost"
#define BIND_APPSRV_SETTING_LOOKING_PORT        ":BindAppsrvSettingLookingPort"

static struct comm_context *g_commctx = NULL;

static void usrapi_o_cb(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;

	x_printf(D, "callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			printf("server here is accept : %d\n", socket);
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
			break;

		default:
			printf("unknow!\n");
			break;
	}
}

int usrapi_o_wrap_init(void)
{
	g_commctx = commapi_ctx_create();

	if (!g_commctx) {
		return -1;
	}

	dictionary      *config = iniparser_load(CONFIG);
	const char            *host = iniparser_getstring(config, BIND_APPSRV_SETTING_LOOKING_HOST, NULL);
	const char            *port = iniparser_getstring(config, BIND_APPSRV_SETTING_LOOKING_PORT, NULL);

	struct comm_cbinfo cbinfo = {};
	cbinfo.monitor = true;
	cbinfo.fcb = usrapi_o_cb;
	assert(commapi_socket(g_commctx, host, port, &cbinfo, COMM_BIND) != -1);

	iniparser_freedict(config);
	return 0;
}

void usrapi_o_wrap_exit(void)
{
	commapi_ctx_destroy(g_commctx);
}

int usrapi_o_wrap_recv(struct comm_message *msg)
{
	assert(msg);
	return commapi_recv(g_commctx, msg);
}

int usrapi_o_wrap_send(struct comm_message *msg)
{
	assert(msg);
	return commapi_send(g_commctx, msg);
}

