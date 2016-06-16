#include "zmq_io_wraper.h"
#include "config_reader.h"
#include "loger.h"

#include <assert.h>

#define CONFIG          "settingServer.conf"
#define CLIENT_IP       "APIIP"
#define CLIENT_PORT     "APIPort"

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
	log("addr:%s.", addr);
	int rc = zmq_connect(g_client, addr);
	log("rc:%d", rc);
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
static void zmq_client_exit()
{
	if (g_client) {
		zmq_close(g_client);
	}
}

void zmq_exit()
{
	zmq_client_exit();
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

int init_zmq_io()
{
	struct config_reader *config =
		init_config_reader(CONFIG);

	assert(!g_ctx);
	g_ctx = zmq_ctx_new();
	char    *clientIp = get_config_name(config, CLIENT_IP);
	char    *clientPort = get_config_name(config, CLIENT_PORT);
	log("clientIp:%s, clientPort:%s", clientIp, clientPort);
	zmq_client_init(clientIp, atoi(clientPort));
	destroy_config_reader(config);
	return 0;
}

