#include "simulate.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

static void *api_server = NULL;
void init_api_server()
{
	api_server = zmq_socket(g_ctx, ZMQ_PUSH);
	int rc = zmq_connect(api_server, "tcp://127.0.0.1:8102");
	assert(rc == 0);
}

void send_to_api(char *str, int flag)
{
	assert(str);
	zmq_msg_t part;
	zmq_msg_init_size(&part, strlen(str));

	memcpy(zmq_msg_data(&part), str, strlen(str));
	zmq_sendmsg(api_server, &part, flag);	// 0 or ZMQ_SNDMORE
}

void destroy_api_server()
{
	zmq_close(api_server);
}

