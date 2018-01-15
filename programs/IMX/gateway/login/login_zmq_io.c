#include <assert.h>

#include "zmq.h"
#include "iniparser.h"
#include "libmini.h"
#include "login_zmq_io.h"


#define CONFIG          "loginGateway.conf"
#define APP_HOST          ":AppHost"
#define APP_PORT        ":AppPort"
#define API_HOST          ":ApiHost"
#define API_PORT        ":ApiPort"

static void     *g_ctx = NULL;
static void     *g_app = NULL;
static void     *g_api = NULL;
static void *zmq_srv_init(char *host, int port)
{
	assert(g_ctx);
	void *sock = zmq_socket(g_ctx, ZMQ_PUSH);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	x_printf(D, "addr:%s.", addr);
	int rc = zmq_bind(sock, addr);
	assert(rc == 0);
	return sock;
}

static void *zmq_cli_init(char *host, int port)
{
	assert(g_ctx);
	void *sock = zmq_socket(g_ctx, ZMQ_PUSH);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	x_printf(D, "addr:%s.", addr);
	int rc = zmq_connect(sock, addr);
	assert(rc == 0);
	return sock;
}

int login_zmq_io_init(void)
{
	assert(!g_ctx);
	g_ctx = zmq_ctx_new();

	dictionary    *config = iniparser_load(CONFIG);
	char    *host = iniparser_getstring(config, APP_HOST, NULL);
	char    *port = iniparser_getstring(config, APP_PORT, NULL);
	g_app = zmq_srv_init(host, atoi(port));

	host = iniparser_getstring(config, API_HOST, NULL);
	port = iniparser_getstring(config, API_PORT, NULL);
	g_api = zmq_cli_init(host, atoi(port));

	iniparser_freedict(config);
	return 0;
}

void login_zmq_io_exit(void)
{
	zmq_close(g_app);
	zmq_close(g_api);
	zmq_ctx_destroy(g_ctx);
}


int login_zmq_io_send_to_app(zmq_msg_t *msg, int flags)
{
	return zmq_sendmsg(g_app, msg, flags);
}

int login_zmq_io_send_to_api(zmq_msg_t *msg, int flags)
{
	return zmq_sendmsg(g_api, msg, flags);
}
