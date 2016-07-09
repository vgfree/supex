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


static int g_conn_types = 0x00000000;

static void *g_skt_upstream     = NULL;
static void *g_skt_downstream   = NULL;
static void *g_skt_status	= NULL;
static void *g_skt_setting	= NULL;
static void *g_skt_looking	= NULL;

void init_connect(void *ctx, int types) {
	int rc = -1;
	char *host;
	char *port;
	char addr[64] = {};
	/*set types*/
	g_conn_types = types;
	printf("g_conn_types : %x\n", g_conn_types);
	assert(g_conn_types > 0);
	/*init socket*/
	struct config_reader *config = init_config_reader(SRV_CONFIG);
	if((g_conn_types & TYPE_UPSTREAM) == TYPE_UPSTREAM) {
		host = get_config_name(config, GATEWAY_PUSH_HOST);
		port = get_config_name(config, GATEWAY_PUSH_PORT);
		snprintf(addr, 63, "tcp://%s:%s", host, port);

		g_skt_upstream = zmq_socket(ctx, ZMQ_PULL);
		rc = zmq_connect(g_skt_upstream, addr);
		printf("connected with upstream\n");
		assert(rc == 0);
	}

	if((g_conn_types & TYPE_DOWNSTREAM) == TYPE_DOWNSTREAM) {
		host = get_config_name(config, GATEWAY_PULL_HOST);
		port = get_config_name(config, GATEWAY_PULL_PORT);
		snprintf(addr, 63, "tcp://%s:%s", host, port);

		g_skt_downstream = zmq_socket(ctx, ZMQ_PUSH);
		rc = zmq_connect(g_skt_downstream, addr);
		printf("connected with downstream\n");
		assert(rc == 0);
	}

	if((g_conn_types & TYPE_STATUS) == TYPE_STATUS) {
		host = get_config_name(config, LOGIN_PUSH_HOST);
		port = get_config_name(config, LOGIN_PUSH_PORT);
		snprintf(addr, 63, "tcp://%s:%s", host, port);

		g_skt_status = zmq_socket(ctx, ZMQ_PULL);
		rc = zmq_connect(g_skt_status, addr);
		printf("connected with loginserver\n");
		assert(rc == 0);
	}

	if((g_conn_types & TYPE_SETTING) == TYPE_SETTING) {
		host = get_config_name(config, USERINFOAPI_PULL_HOST);
		port = get_config_name(config, USERINFOAPI_PULL_PORT);
		snprintf(addr, 63, "tcp://%s:%s", host, port);

		g_skt_setting = zmq_socket(ctx, ZMQ_PUSH);
		rc = zmq_connect(g_skt_setting, addr);
		printf("connected with userinfoapi\n");
		assert(rc == 0);
	}
	//TODO:
	destroy_config_reader(config);
}

void destroy_connect(void)
{
	if(g_skt_upstream) {
		zmq_close(g_skt_upstream);
	}
	if(g_skt_downstream) {
		zmq_close(g_skt_downstream);
	}
	if(g_skt_status) {
		zmq_close(g_skt_status);
	}
	if(g_skt_setting) {
		zmq_close(g_skt_setting);
	}
	if(g_skt_looking) {
		zmq_close(g_skt_looking);
	}
}

#define RECV_UPSTREAM_AND_LOGIN     0x00000101
int recv_msg(struct app_msg *msg, int* more, int flag) {
	int rc = -1;
	if((g_conn_types & RECV_UPSTREAM_AND_LOGIN) == RECV_UPSTREAM_AND_LOGIN) {
		rc = recv_more_msg(msg, g_skt_upstream, g_skt_status, more, flag);	
	}else if((g_conn_types & RECV_UPSTREAM) == RECV_UPSTREAM) {
		rc = recv_gateway_msg(msg, g_skt_upstream, ZMQ_DONTWAIT);

	}else if((g_conn_types & TYPE_STATUS) == TYPE_STATUS) {
		rc = recv_login_msg(msg, g_skt_status, ZMQ_DONTWAIT);

	}
#if 0
	int i;
	//TODO check connect type and connectors
	for(i = 0; i < connectors -> count; i++) {
		if(memcmp(connectors -> container[i], "g_skt_upstream", 
					18) == 0) {
			int rc = recv_gateway_msg(msg, flag);
		}
		if(memcmp(connectors -> container[i], "g_skt_status",
					20) == 0) {
			int rc = recv_login_msg(msg, flag);
		}
	}	
#endif
	return rc;
}

int send_msg(struct app_msg *msg) {
	int rc = -1;
	if((g_conn_types & TYPE_SETTING) == TYPE_SETTING) {
		printf("send to userinfoapi\n");
		rc = send_to_api(msg, g_skt_setting);
	}
	
	if((g_conn_types & TYPE_DOWNSTREAM) == TYPE_DOWNSTREAM) {
		printf("send to message_gateway(client)\n");
		rc = send_to_gateway(msg, g_skt_downstream);
	}
	return rc;
}
