#include "iniparser.h"
#include "libmini.h"
#include "control_sockfd_manage.h"

#include "control_comm_io.h"

#define CONFIG                  "controlGateway.conf"
#define NODE_SERVER_HOST        ":bind_exchage_pull_setting_host"
#define NODE_SERVER_PORT        ":bind_exchage_pull_setting_port"

static struct comm_context *g_commctx = NULL;


static void core_exchange_node_cb(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	x_printf(D, "callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			printf("server here is accept : %d\n", socket);
			control_sockfd_manage_add(socket);
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
			control_sockfd_manage_del(socket);
			break;

		default:
			printf("unknow!\n");
			break;
	}
}

int control_comm_io_init(void)
{
	dictionary    *config = iniparser_load(CONFIG);
	char    *host = iniparser_getstring(config, NODE_SERVER_HOST, NULL);
	char    *port = iniparser_getstring(config, NODE_SERVER_PORT, NULL);
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


void control_comm_io_exit(void)
{
	commapi_ctx_destroy(g_commctx);
}

int control_comm_io_recv(struct comm_message *msg)
{
	assert(msg);
	return commapi_recv(g_commctx, msg);
}

int control_comm_io_send(struct comm_message *msg)
{
	assert(msg);
	return commapi_send(g_commctx, msg);
}

