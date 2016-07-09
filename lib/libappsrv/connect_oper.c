/**
 * author:lanjian
 * date:2016/07/08
 */
#include "config_reader.h"
#include "zmq.h"
#include "recv_wraper.h"
#include "send_wraper.h"
#include <assert.h>
#include <memory.h>
#include "connect_oper.h"

#define SRV_CONFIG                  "libappsrv.conf"

#define RECV_UPSTREAM               0x00000001
#define RECV_GATEWAY_IP             "recv_gateway_ip"
#define RECV_GATEWAY_PORT           "recv_gateway_port"


#define SEND_DOWNSTREAM             0x00000010
#define SEND_GATEWAY_IP             "send_gateway_ip"
#define SEND_GATEWAY_PORT           "send_gateway_port"

#define RECV_LOGIN                  0x00000100
#define RECV_LOGIN_IP               "recv_login_ip"
#define RECV_LOGIN_PORT             "recv_login_port"

#define SEND_SETTING                0x00001000
#define SEND_USERINFOAPI_IP         "send_userinfoapi_ip"
#define SEND_USERINFOAPI_PORT       "send_userinfoapi_port"

#define RECV_UPSTREAM_AND_LOGIN     0x00000101

static void *ct_upstream     = NULL;
static void *ct_downstream   = NULL;
static void *ct_recv_login   = NULL;
static void *ct_send_setting = NULL;
static int conn_type = 0x00000000;

void init_connect(void *ctx, ct_type connect_type) {
	int rc = -1;
	char *ip;
	char *port;
	char addr[64] = {};
	struct config_reader *config = init_config_reader(SRV_CONFIG);
	conn_type = connect_type;
	printf("conn_type : %x\n", conn_type);
	assert(conn_type > 0);

	if((conn_type & RECV_UPSTREAM) == RECV_UPSTREAM) {
		ip = get_config_name(config, RECV_GATEWAY_IP);
		port = get_config_name(config, RECV_GATEWAY_PORT);
		snprintf(addr, 63, "tcp://%s:%s", ip, port);
		ct_upstream = zmq_socket(ctx, ZMQ_PULL);
		rc = zmq_connect(ct_upstream, addr);
		printf("connected with upstream\n");
		assert(rc == 0);
	}

	if((conn_type & SEND_DOWNSTREAM) == SEND_DOWNSTREAM) {
		ip = get_config_name(config, SEND_GATEWAY_IP);
		port = get_config_name(config, SEND_GATEWAY_PORT);
		snprintf(addr, 63, "tcp://%s:%s", ip, port);
		ct_downstream = zmq_socket(ctx, ZMQ_PUSH);
		rc = zmq_connect(ct_downstream, addr);
		printf("connected with downstream\n");
		assert(rc == 0);
	}

	if((conn_type & RECV_LOGIN) == RECV_LOGIN) {
		ip = get_config_name(config, RECV_LOGIN_IP);
		port = get_config_name(config, RECV_LOGIN_PORT);
		snprintf(addr, 63, "tcp://%s:%s", ip, port);
		ct_recv_login = zmq_socket(ctx, ZMQ_PULL);
		rc = zmq_connect(ct_recv_login, addr);
		printf("connected with loginserver\n");
		assert(rc == 0);
	}

	if((conn_type & SEND_SETTING) == SEND_SETTING) {
		ip = get_config_name(config, SEND_USERINFOAPI_IP);
		port = get_config_name(config, SEND_USERINFOAPI_PORT);
		snprintf(addr, 63, "tcp://%s:%s", ip, port);
		ct_send_setting = zmq_socket(ctx, ZMQ_PULL);
		rc = zmq_connect(ct_send_setting, addr);
		printf("connected with userinfoapi\n");
		assert(rc == 0);
	}
	destroy_config_reader(config);
}

void destroy_connect() {
	if(ct_upstream) {
		zmq_close(ct_upstream);
	}
	if(ct_downstream) {
		zmq_close(ct_downstream);
	}
	if(ct_recv_login) {
		zmq_close(ct_recv_login);
	}
	if(ct_send_setting) {
		zmq_close(ct_send_setting);
	}
}

int recv_msg(struct app_msg *msg, int* more, int flag) {
	int rc = -1;
	if((conn_type & RECV_UPSTREAM_AND_LOGIN) == RECV_UPSTREAM_AND_LOGIN) {
		rc = recv_more_msg(msg, ct_upstream, ct_recv_login, more, flag);	
	}else if((conn_type & RECV_UPSTREAM) == RECV_UPSTREAM) {
		rc = recv_gateway_msg(msg, ct_upstream, ZMQ_DONTWAIT);

	}else if((conn_type & RECV_LOGIN) == RECV_LOGIN) {
		rc = recv_login_msg(msg, ct_recv_login, ZMQ_DONTWAIT);

	}
#if 0
	int i;
	//TODO check connect type and connectors
	for(i = 0; i < connectors -> count; i++) {
		if(memcmp(connectors -> container[i], "ct_upstream", 
					18) == 0) {
			int rc = recv_gateway_msg(msg, flag);
		}
		if(memcmp(connectors -> container[i], "ct_recv_login",
					20) == 0) {
			int rc = recv_login_msg(msg, flag);
		}
	}	
#endif
	return rc;
}

int send_msg(struct app_msg *msg) {
	int rc = -1;
	if((conn_type & SEND_SETTING) == SEND_SETTING) {
		printf("send to userinfoapi\n");
		rc = send_to_api(msg, ct_send_setting);
	}
	
	if((conn_type & SEND_DOWNSTREAM) == SEND_DOWNSTREAM) {
		printf("send to message_gateway(client)\n");
		rc = send_to_gateway(msg, ct_downstream);
	}
	return rc;
}
