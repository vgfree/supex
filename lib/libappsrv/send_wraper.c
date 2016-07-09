#include "config_reader.h"
#include "send_wraper.h"

#include <assert.h>
#include <memory.h>

#if 0
#define CONFIG                  "libappsrv.conf"
#define PUSH_API_IP             "PushApiIP"
#define PUSH_API_PORT           "PushApiPort"
#define PUSH_GATEWAY_IP         "PushGatewayIP"
#define PUSH_GATEWAY_PORT       "PushGatewayPort"
#define SEND_STRATEGY           "AppServSendStrategy"
static void     *s_api = NULL;
static void     *s_gateway = NULL;
static void     *send_strategy = NULL;

void init_send(void *ctx)
{
	assert(!s_api && !s_gateway && !send_strategy);
	struct config_reader *config = init_config_reader(CONFIG);
	send_strategy = get_config_name(config, SEND_STRATEGY); 
	int rc = -1;
	char *ip;
	char *port;
	char addr[64] = {};
	if(memcmp(send_strategy, "all", 3) == 0) {
		ip = get_config_name(config, PUSH_API_IP);
		port = get_config_name(config, PUSH_API_PORT);
		snprintf(addr, 63, "tcp://%s:%s", ip, port);
		s_api = zmq_socket(ctx, ZMQ_PUSH);
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
	}else if(memcmp(send_strategy, "onlyUserinfoApi", 15) == 0) {
		ip = get_config_name(config, PUSH_API_IP);
		port = get_config_name(config, PUSH_API_PORT);
		snprintf(addr, 63, "tcp://%s:%s", ip, port);
		s_api = zmq_socket(ctx, ZMQ_PUSH);
		int rc = zmq_connect(s_api, addr);
		assert(rc == 0);
		destroy_config_reader(config);
	}else if(memcmp(send_strategy, "onlyGateway", 11) == 0) {
		ip = get_config_name(config, PUSH_GATEWAY_IP);
		port = get_config_name(config, PUSH_GATEWAY_PORT);
		memset(addr, 0, 64);
		snprintf(addr, 63, "tcp://%s:%s", ip, port);
		s_gateway = zmq_socket(ctx, ZMQ_PUSH);
		rc = zmq_connect(s_gateway, addr);
		assert(rc == 0);
		destroy_config_reader(config);
	} 

}

void destroy_send()
{
	assert(s_api && s_gateway && send_strategy);
	if(memcmp(send_strategy, "all", 3) == 0) {
		zmq_close(s_api);
		zmq_close(s_gateway);
	}else if(memcmp(send_strategy, "onlyUserinfoApi", 15) == 0) {
		zmq_close(s_api);
	}else if(memcmp(send_strategy, "onlyGateway", 11) == 0) {
		zmq_close(s_gateway);
	}
}
#endif

int send_to_api(struct app_msg *msg, void *ct)
{
	assert(msg && ct);
	return zmq_sendiov(ct, msg->vector, msg->vector_size, ZMQ_SNDMORE | ZMQ_DONTWAIT);
}

int send_to_gateway(struct app_msg *msg, void *ct)
{
	assert(msg && ct);
	return zmq_sendiov(ct, msg->vector, msg->vector_size, ZMQ_SNDMORE | ZMQ_DONTWAIT);
}

