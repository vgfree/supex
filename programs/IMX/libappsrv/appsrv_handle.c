/**
 * author:lanjian
 * date:2016/07/08
 */
#include "iniparser.h"
#include "zmq.h"
#include <assert.h>
#include <memory.h>
#include "appsrv_handle.h"


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
	assert(g_conn_types > 0);
	/*init socket*/
	dictionary    *config = iniparser_load(SRV_CONFIG);
	if((g_conn_types & TYPE_UPSTREAM) == TYPE_UPSTREAM) {
		host = iniparser_getstring(config, GATEWAY_PUSH_HOST, NULL);
		port = iniparser_getstring(config, GATEWAY_PUSH_PORT, NULL);
		snprintf(addr, 63, "tcp://%s:%s", host, port);

		g_skt_upstream = zmq_socket(ctx, ZMQ_PULL);
		rc = zmq_connect(g_skt_upstream, addr);
		printf("connected with upstream\n");
		assert(rc == 0);
	}

	if((g_conn_types & TYPE_DOWNSTREAM) == TYPE_DOWNSTREAM) {
		host = iniparser_getstring(config, GATEWAY_PULL_HOST, NULL);
		port = iniparser_getstring(config, GATEWAY_PULL_PORT, NULL);
		snprintf(addr, 63, "tcp://%s:%s", host, port);

		g_skt_downstream = zmq_socket(ctx, ZMQ_PUSH);
		rc = zmq_connect(g_skt_downstream, addr);
		printf("connected with downstream\n");
		assert(rc == 0);
	}

	if((g_conn_types & TYPE_STATUS) == TYPE_STATUS) {
		host = iniparser_getstring(config, LOGIN_PUSH_HOST, NULL);
		port = iniparser_getstring(config, LOGIN_PUSH_PORT, NULL);
		snprintf(addr, 63, "tcp://%s:%s", host, port);

		g_skt_status = zmq_socket(ctx, ZMQ_PULL);
		rc = zmq_connect(g_skt_status, addr);
		printf("connected with loginserver\n");
		assert(rc == 0);
	}

	if((g_conn_types & TYPE_SETTING) == TYPE_SETTING) {
		host = iniparser_getstring(config, USERINFOAPI_PULL_HOST, NULL);
		port = iniparser_getstring(config, USERINFOAPI_PULL_PORT, NULL);
		snprintf(addr, 63, "tcp://%s:%s", host, port);

		g_skt_setting = zmq_socket(ctx, ZMQ_PUSH);
		rc = zmq_connect(g_skt_setting, addr);
		printf("connected with userinfoapi to setting\n");
		assert(rc == 0);
	}
	if((g_conn_types & TYPE_LOOKING) == TYPE_LOOKING) {
		host = iniparser_getstring(config, USERINFOAPI_REP_HOST, NULL);
		port = iniparser_getstring(config, USERINFOAPI_REP_PORT, NULL);
		snprintf(addr, 63, "tcp://%s:%s", host, port);

		g_skt_looking = zmq_socket(ctx, ZMQ_REQ);
		rc = zmq_connect(g_skt_looking, addr);
		printf("connected with userinfoapi to looking\n");
		assert(rc == 0);
	}
	iniparser_freedict(config);
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

int recv_msg(enum askt_type type, struct app_msg *msg) {
	int rc = -1;
	msg->vector_size = MAX_SPILL_DEPTH;
	switch(type) {
		case TYPE_UPSTREAM:
			rc = zmq_recviov(g_skt_upstream, msg->vector, 
					&msg->vector_size, 0);
			break;
		case TYPE_STATUS:
			rc = zmq_recviov(g_skt_status, msg->vector, 
					&msg->vector_size, 0);
			break;
		case TYPE_LOOKING:
			rc = zmq_recviov(g_skt_looking, msg->vector, 
					&msg->vector_size, 0);
		default:
			break;
	}

	return rc;
}

int recv_more_msg(int types, struct app_msg *msg, int flag) {
	int rc = -1;
	zmq_pollitem_t items[3];
	int i = 0;
	if((types & TYPE_STATUS) == TYPE_STATUS) {
		items[i].socket = g_skt_status;
		items[i].events = ZMQ_POLLIN;
		i ++;
	}
	if((types & TYPE_LOOKING) == TYPE_LOOKING) {
		items[i].socket = g_skt_looking;
		items[i].events = ZMQ_POLLIN;
		i ++;
	}
	if((types & TYPE_UPSTREAM) == TYPE_UPSTREAM) {
		items[i].socket = g_skt_upstream;  
		items[i].events = ZMQ_POLLIN;
		i ++;
	}
	rc = zmq_poll(items, i, flag);	// -1, block, 0,not block.
	assert(rc >= 0);

	msg->vector_size = MAX_SPILL_DEPTH;
	int j;
	for(j = 0; j < i; j++) {
		if(items[j].revents > 0) {
			return zmq_recviov(items[j].socket, msg->vector, 
					&msg->vector_size, 0);
		}
	}

	return rc;
}

int send_msg(enum askt_type type, struct app_msg *msg) {
	int rc = -1;

	switch(type) {
		case TYPE_DOWNSTREAM:
			rc = zmq_sendiov(g_skt_downstream, msg->vector, msg->vector_size, 
					ZMQ_SNDMORE | ZMQ_DONTWAIT);
			break;
		case TYPE_SETTING:
			rc = zmq_sendiov(g_skt_setting, msg->vector, msg->vector_size, 
					ZMQ_SNDMORE | ZMQ_DONTWAIT);
			break;
		case TYPE_LOOKING:
			rc = zmq_sendiov(g_skt_looking, msg->vector, msg->vector_size, 
					ZMQ_SNDMORE | ZMQ_DONTWAIT);
		default:
			break;
	}

	return rc;
}
