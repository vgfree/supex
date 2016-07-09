#include "config_reader.h"
#include "recv_wraper.h"
#include "zmq.h"

#include <assert.h>
#include <memory.h>
#if 0
#define CONFIG                  "libappsrv.conf"
#define PULL_LOGIN_IP           "PullLoginIP"
#define PULL_LOGIN_PORT         "PullLoginPort"
#define PULL_GATEWAY_IP         "PullGatewayIP"
#define PULL_GATEWAY_PORT       "PullGatewayPort"
#define RECV_STRATEGY           "AppServRecvStrategy"

static void     *s_gateway = NULL;
static void     *s_login = NULL;
static char     *recv_strategy = NULL;
void init_recv(void *ctx)
{
	assert(!s_gateway && !s_login && !recv_strategy);
	struct config_reader *config = init_config_reader(CONFIG);
	recv_strategy = get_config_name(config, RECV_STRATEGY); 
	int rc = -1;
	char *ip;
	char *port;
	char addr[64] = {};
	if(memcmp(recv_strategy, "all", 3) == 0) {
		ip = get_config_name(config, PULL_LOGIN_IP);
		port = get_config_name(config, PULL_LOGIN_PORT);
		snprintf(addr, 63, "tcp://%s:%s", ip, port);
		s_login = zmq_socket(ctx, ZMQ_PULL);
		int rc = zmq_connect(s_login, addr);
		assert(rc == 0);

		ip = get_config_name(config, PULL_GATEWAY_IP);
		port = get_config_name(config, PULL_GATEWAY_PORT);
		memset(addr, 0, 64);
		snprintf(addr, 63, "tcp://%s:%s", ip, port);
		s_gateway = zmq_socket(ctx, ZMQ_PULL);
		rc = zmq_connect(s_gateway, addr);	
		assert(rc == 0);
	}else if(memcmp(recv_strategy, "onlyLogin", 9) == 0) {
		ip = get_config_name(config, PULL_LOGIN_IP);
		port = get_config_name(config, PULL_LOGIN_PORT);
		snprintf(addr, 63, "tcp://%s:%s", ip, port);
		s_login = zmq_socket(ctx, ZMQ_PULL);
		int rc = zmq_connect(s_login, addr);
		assert(rc == 0);
	}else if(memcmp(recv_strategy, "onlyGateway", 11) == 0){
		ip = get_config_name(config, PULL_GATEWAY_IP);
		port = get_config_name(config, PULL_GATEWAY_PORT);
		snprintf(addr, 63, "tcp://%s:%s", ip, port);
		s_gateway = zmq_socket(ctx, ZMQ_PULL);
		rc = zmq_connect(s_gateway, addr);	
		assert(rc == 0);
	}
}

void destroy_recv()
{
	assert(s_login && s_gateway && recv_strategy);
	if(memcmp(recv_strategy, "all", 3) == 0) {
		zmq_close(s_login);
		zmq_close(s_gateway);
	}else if(memcmp(recv_strategy, "onlyLogin", 9) == 0) {
		zmq_close(s_login);
	}else if(memcmp(recv_strategy, "onlyGateway", 11) == 0) {
		zmq_close(s_gateway);
	}
}
#endif

/**
 * flag: 0 means block, ZMQ_DONTWAIT means not block
 */
int recv_login_msg(struct app_msg *msg, void *ct_recv_login, int flag) {
	assert(msg);
//	int rc = zmq_msg_recv(msg, ct_recv_login, flag);
//	assert (rc != -1);

	msg->vector_size = MAX_SPILL_DEPTH;
	return zmq_recviov(ct_recv_login, msg->vector, &msg->vector_size, 0);
}

/**
 * flag: 0 means block, ZMQ_DONTWAIT means not block
 */
int recv_gateway_msg(struct app_msg *msg, void *ct_upstream, int flag) {
	assert(msg);
//	int rc = zmq_msg_recv(msg, ct_upstream, flag);
//	assert (rc != -1);

	msg->vector_size = MAX_SPILL_DEPTH;
	return zmq_recviov(ct_upstream, msg->vector, &msg->vector_size, 0);
}

int recv_more_msg(struct app_msg *msg, void *ct_upstream, void *ct_recv_login,
		int flag)
{
	assert(msg && more);
	zmq_pollitem_t items[2];
	int i = 0;
	if((types & TYPE_UPSTREAM) == TYPE_UPSTREAM) {
		items[i].socket = ct_upstream;
		items[i].events = ZMQ_POLLIN;
		i ++;
	}
	if((types & TYPE_STATUS) == TYPE_STATUS) {
		items[i].socket = ct_recv_login;
		items[i].events = ZMQ_POLLIN;
		i ++;
	}
	if((types & TYPE_LOOKING) == TYPE_LOOKING) {
		items[i].socket = ct_recv_login;
		items[i].events = ZMQ_POLLIN;
		i ++;
	}
	int rc = zmq_poll(items, i, flag);	// -1, block, 0,not block.
	assert(rc >= 0);


	msg->vector_size = MAX_SPILL_DEPTH;

	if (items[0].revents > 0) {
		return zmq_recviov(items[i].socket, msg->vector, &msg->vector_size, 0);
	} else if (items[1].revents > 0) {
		return zmq_recviov(ct_upstream, msg->vector, &msg->vector_size, 0);
	}

	return rc;
}

