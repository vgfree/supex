#include <assert.h>

#include "zmq.h"
#include "iniparser.h"
#include "libmini.h"
#include "control_zmq_io.h"

#define CONFIG          "controlGateway.conf"
#define CLIENT_HOST       ":APIHost"
#define CLIENT_PORT     ":APIPort"

static void *g_ctx = NULL;
static void *g_srv;
static void *zmq_srv_init(char *host, int port)
{
	assert(g_ctx);
	void *sock = zmq_socket(g_ctx, ZMQ_PULL);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	x_printf(D, "addr:%s.", addr);
	int rc = zmq_bind(sock, addr);
	assert(rc == 0);
	return sock;
}


int control_zmq_io_init(void)
{
	assert(!g_ctx);
	g_ctx = zmq_ctx_new();

	dictionary    *config = iniparser_load(CONFIG);
	char    *host = iniparser_getstring(config, CLIENT_HOST, NULL);
	char    *port = iniparser_getstring(config, CLIENT_PORT, NULL);
	g_srv = zmq_srv_init(host, atoi(port));

	iniparser_freedict(config);
	return 0;
}


void control_zmq_io_exit(void)
{
	zmq_close(g_srv);
	zmq_ctx_destroy(g_ctx);
}

int control_zmq_io_recv(zmq_msg_t *msg, int flags)
{
	return zmq_recvmsg(g_srv, msg, flags);
}

int control_zmq_io_getsockopt(enum zio_rw_type rwopt, int option_name, void *option_value, size_t *option_len)
{
	return zmq_getsockopt((rwopt == ZIO_RECV_TYPE) ? g_srv : g_srv, option_name, option_value, option_len);
}

