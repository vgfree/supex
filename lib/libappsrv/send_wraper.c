#include "config_reader.h"
#include "send_wraper.h"

#include <assert.h>
#include <memory.h>

#define CONFIG                  "libappsrv.conf"
#define PUSH_API_IP             "PushApiIP"
#define PUSH_API_PORT           "PushApiPort"
#define PUSH_GATEWAY_IP         "PushGatewayIP"
#define PUSH_GATEWAY_PORT       "PushGatewayPort"
static void     *s_api = NULL;
static void     *s_gateway = NULL;

void init_send(void *ctx)
{
	assert(!s_api && !s_gateway);
	struct config_reader *config = init_config_reader(CONFIG);
	s_api = zmq_socket(ctx, ZMQ_PUSH);
	char    *ip = get_config_name(config, PUSH_API_IP);
	char    *port = get_config_name(config, PUSH_API_PORT);
	char    addr[64] = {};
	snprintf(addr, 63, "tcp://%s:%s", ip, port);
	int rc = zmq_connect(s_api, addr);
	assert(rc == 0);

	ip = get_config_name(config, PUSH_GATEWAY_IP);
	port = get_config_name(config, PUSH_GATEWAY_PORT);
	memset(addr, 0, 64);
	snprintf(addr, 63, "tcp://%s:%s", ip, port);
	s_gateway = zmq_socket(ctx, ZMQ_PUSH);
	rc = zmq_connect(s_gateway, addr);
	assert(rc == 0);
	destroy_config_reader(config);
}

void destroy_send()
{
	assert(s_api && s_gateway);
	zmq_close(s_api);
	zmq_close(s_gateway);
}

int send_to_api(struct app_msg *msg)
{
	assert(msg && s_api);
	return zmq_sendiov(s_api, msg->vector, msg->vector_size, ZMQ_SNDMORE | ZMQ_DONTWAIT);
}

int send_to_gateway(struct app_msg *msg)
{
	assert(msg && s_gateway);
	return zmq_sendiov(s_gateway, msg->vector, msg->vector_size, ZMQ_SNDMORE | ZMQ_DONTWAIT);
}

