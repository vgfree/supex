#include "config_reader.h"
#include "zmq_io_wraper.h"
#include "libmini.h"

#include <assert.h>

#define CONFIG          "loginServer.conf"
#define APP_HOST          "AppHost"
#define APP_PORT        "AppPort"
#define API_HOST          "ApiHost"
#define API_PORT        "ApiPort"

static void     *g_ctx = NULL;
static void     *g_server = NULL;
static void     *g_api = NULL;
static void zmq_srv_init(char *host, int port)
{
	assert(g_ctx);
	g_server = zmq_socket(g_ctx, ZMQ_PUSH);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	x_printf(D, "addr:%s.", addr);
	int rc = zmq_bind(g_server, addr);
	assert(rc == 0);
}

static void zmq_cli_init(char *host, int port)
{
	assert(g_ctx);
	g_api = zmq_socket(g_ctx, ZMQ_PUSH);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	x_printf(D, "addr:%s.", addr);
	int rc = zmq_connect(g_api, addr);
	assert(rc == 0);
}


void exit_zmq_io(void)
{
	zmq_close(g_server);
	zmq_close(g_api);
	zmq_ctx_destroy(g_ctx);
}

int zmq_io_send_app(zmq_msg_t *msg, int flags)
{
	return zmq_sendmsg(g_server, msg, flags);
}

int zmq_io_send_api(zmq_msg_t *msg, int flags)
{
	return zmq_sendmsg(g_api, msg, flags);
}

int init_zmq_io(void)
{
	assert(!g_ctx);
	g_ctx = zmq_ctx_new();

	struct config_reader *config = init_config_reader(CONFIG);
	char    *host = get_config_name(config, APP_HOST);
	char    *port = get_config_name(config, APP_PORT);
	zmq_srv_init(host, atoi(port));

	host = get_config_name(config, API_HOST);
	port = get_config_name(config, API_PORT);
	zmq_cli_init(host, atoi(port));

	destroy_config_reader(config);
	return 0;
}

