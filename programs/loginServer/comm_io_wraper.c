#include "comm_io_wraper.h"
#include "config_reader.h"
#include "libmini.h"

#define CONFIG                  "loginServer.conf"
#define NODE_SERVER_HOST        "NodeServer"
#define NODE_SERVER_PORT        "NodePort"

static void core_exchange_node_cb(struct comm_context   *commctx,
	struct comm_tcp                                 *portinfo,
	void                                            *usr)
{
	x_printf(D, "callback, fd:%d, status:%d.", portinfo->fd, portinfo->stat);

	if (portinfo->stat == FD_INIT) {
		if (g_node_ptr->max_size > NODE_SIZE) {
			x_printf(E, "core exchange node:%d > max size:%d.", g_node_ptr->max_size, NODE_SIZE);
		}

		g_node_ptr->fd_array[g_node_ptr->max_size++] = portinfo->fd;
	} else if (portinfo->stat == FD_CLOSE) {
		int i = 0;
		for (; i < g_node_ptr->max_size; i++) {
			if (g_node_ptr->fd_array[i] == portinfo->fd) {
				break;
			}
		}

		for (; i < g_node_ptr->max_size - 1; i++) {
			g_node_ptr->fd_array[i] = g_node_ptr->fd_array[i + 1];
		}

		if (g_node_ptr->max_size == i) {
			x_printf(E, "not g_node fd_array no include this fd:%d.", portinfo->fd);
		} else {
			g_node_ptr->max_size--;
		}
	}
}

static struct comm_context *g_commctx = NULL;
int init_comm_io(void)
{
	g_node_ptr = (struct core_exchange_node *)malloc(sizeof(struct core_exchange_node));
	g_node_ptr->max_size = 0;

	struct config_reader *config = init_config_reader(CONFIG);
	char    *host = get_config_name(config, NODE_SERVER_HOST);
	char    *port = get_config_name(config, NODE_SERVER_PORT);
	x_printf(D, "nodeServer:%s, nodePort:%s.", host, port);

	g_commctx = commapi_ctx_create();
	if (!g_commctx) {
		return -1;
	}

	struct cbinfo callback_info = {};
	callback_info.callback = core_exchange_node_cb;
	assert(commapi_socket(g_commctx, host, port, &callback_info, COMM_BIND) != -1);

	destroy_config_reader(config);
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

