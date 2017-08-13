#include "zmq_io_wraper.h"
#include "iniparser.h"
#include "libmini.h"

#include <assert.h>

#define CONFIG          "settingServer.conf"
#define CLIENT_HOST       ":APIHost"
#define CLIENT_PORT     ":APIPort"

static void *g_ctx = NULL;
// static void *g_server[SERVER_SIZE] = {NULL, NULL, NULL, NULL};
// static void *g_client[CLIENT_SIZE] = {NULL, NULL, NULL, NULL};
static void *g_client;

/*static void zmq_srv_init(char *host, int port, enum server srv)
 *   {
 *   assert(g_ctx);
 *   g_server[srv] = zmq_socket(g_ctx, ZMQ_PUSH);
 *   char addr[64] = {};
 *   sprintf(addr, "tcp://%s:%d", host, port);
 *   int rc = zmq_bind(g_server[srv], addr);
 *   assert(rc == 0);
 *   }*/
static int zmq_client_init(char *host, int port)
{
	assert(g_ctx);
	g_client = zmq_socket(g_ctx, ZMQ_PULL);
	char addr[64] = {};
	sprintf(addr, "tcp://%s:%d", host, port);
	x_printf(D, "addr:%s.", addr);
	int rc = zmq_connect(g_client, addr);
	x_printf(D, "rc:%d", rc);
	return rc;
}

/*
 *   static void zmq_srv_exit()
 *   {
 *   for (int i = 0; i < SERVER_SIZE; i++) {
 *    if (g_server[i]) {
 *      zmq_close(g_server[i]);
 *    }
 *   }
 *   }*/

void exit_zmq_io(void)
{
	if (g_client) {
		zmq_close(g_client);
	}
	//  zmq_srv_exit();
	zmq_ctx_destroy(g_ctx);
}

/*
 *   int zmq_io_send(enum server srv, zmq_msg_t *msg, int flags)
 *   {
 *   return zmq_sendmsg(g_server[srv], msg, flags);
 *   }*/
int zmq_io_getsockopt(int option_name, void *option_value, size_t *option_len)
{
	return zmq_getsockopt(g_client, option_name, option_value, option_len);
}

int zmq_io_recv(zmq_msg_t *msg, int flags)
{
	return zmq_recvmsg(g_client, msg, flags);
}

int init_zmq_io(void)
{
	assert(!g_ctx);
	g_ctx = zmq_ctx_new();
	
	dictionary    *config = iniparser_load(CONFIG);
	char    *host = iniparser_getstring(config, CLIENT_HOST, NULL);
	char    *port = iniparser_getstring(config, CLIENT_PORT, NULL);
	x_printf(D, "clientIp:%s, clientPort:%s", host, port);

	zmq_client_init(host, atoi(port));

	iniparser_freedict(config);
	return 0;
}

