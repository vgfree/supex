#include "comm_io_wraper.h"
#include "config_reader.h"
#include "libmini.h"

#define CONFIG                  "messageGateway.conf"
#define NODE_CONNECT_HOST         "CoreExchangeNodeHost"
#define NODE_CONNECT_PORT       "CoreExchangeNodePort"
#define NODE_SERVER_HOST          "NodeServer"
#define NODE_SERVER_PORT        "NodePort"

static void core_exchange_node_cb(void *ctx, int socket, enum STEP_CODE step, void *usr)
{
	struct comm_context *commctx = (struct comm_context *)ctx;
	x_printf(D, "callback, fd:%d, status:%d.", socket, step);

	switch (step)
	{
		case STEP_INIT:
			printf("server here is connect : %d\n", socket);
			if (g_node_ptr->max_size > NODE_SIZE) {
				x_printf(E, "core exchange node:%d > max size:%d.", g_node_ptr->max_size, NODE_SIZE);
			}

			g_node_ptr->fd_array[g_node_ptr->max_size++] = socket;
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
			int i = 0;
			for (; i < g_node_ptr->max_size; i++) {
				if (g_node_ptr->fd_array[i] == socket) {
					break;
				}
			}

			for (; i < g_node_ptr->max_size - 1; i++) {
				g_node_ptr->fd_array[i] = g_node_ptr->fd_array[i + 1];
			}

			if (g_node_ptr->max_size == i) {
				x_printf(E, "not g_node fd_array no include this fd:%d.", socket);
			} else {
				g_node_ptr->max_size--;
			}
			break;

		default:
			printf("unknow!\n");
			break;
	}
}

static struct comm_context *g_commctx = NULL;

int init_comm_io(void)
{
	dictionary    *config = iniparser_load(CONFIG);
	char    *host = iniparser_getstring(config, NODE_CONNECT_HOST, NULL);
	char    *port = iniparser_getstring(config, NODE_CONNECT_PORT, NULL);

	x_printf(D, "host:%s. port:%s.", host, port);
	g_commctx = commapi_ctx_create();
	if (!g_commctx) {
		return -1;
	}

	struct comm_cbinfo callback_info = {};
	callback_info.fcb = core_exchange_node_cb;

	char    *nodeServer = iniparser_getstring(config, NODE_SERVER_HOST, NULL);
	char    *nodePort = iniparser_getstring(config, NODE_SERVER_PORT, NULL);
	x_printf(D, "nodeServer:%s, nodePort:%s.", nodeServer, nodePort);
	commapi_socket(g_commctx, nodeServer, nodePort, &callback_info, COMM_BIND);
	iniparser_freedict(config);
	return 0;
}

void exit_comm_io(void)
{
	free(g_node_ptr);
}

int recv_msg(struct comm_message *msg)
{
	assert(msg);
	return commapi_recv(g_commctx, msg);
}

int send_msg(struct comm_message *msg)
{
	assert(msg);
	return commapi_send(g_commctx, msg);
}

