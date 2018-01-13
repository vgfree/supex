#include "zmq_warper.h"

#include "iniparser.h"
#include "libmini.h"

#define CONFIG          "messageGateway.conf"
#define PUSH_HOST         ":PushServer"
#define PUSH_PORT       ":PushPort"
#define PULL_HOST         ":PullServer"
#define PULL_PORT       ":PullPort"

static void     *g_ctx = NULL;
static void     *g_push = NULL;
static void     *g_pull = NULL;

int message_zmq_io_init(void)
{
	assert(!g_ctx);
	g_ctx = zmq_ctx_new();
	dictionary    *config = iniparser_load(CONFIG);
	
	char    *host = iniparser_getstring(config, PUSH_HOST, NULL);
	char    *port = iniparser_getstring(config, PUSH_PORT, NULL);
	g_push = zmq_push_init(g_ctx, host, atoi(port));

	host = iniparser_getstring(config, PULL_HOST, NULL);
	port = iniparser_getstring(config, PULL_PORT, NULL);
	g_pull = zmq_pull_init(g_ctx, host, atoi(port));

	iniparser_freedict(config);
	return 0;
}

void message_zmq_io_exit(void)
{
	zmq_close(g_pull);
	zmq_close(g_push);
	zmq_ctx_destroy(g_ctx);
}


int message_zmq_io_send(zmq_msg_t *msg, int flags)
{
	return zmq_sendmsg(g_push, msg, flags);
}

int message_zmq_io_recv(zmq_msg_t *msg, int flags)
{
	return zmq_recvmsg(g_pull, msg, flags);
}

int message_zmq_io_getsockopt(enum zio_rw_type rwopt, int option_name, void *option_value, size_t *option_len)
{
	return zmq_getsockopt((zio_rw_type == ZIO_RECV_TYPE) ? g_pull : g_push, option_name, option_value, option_len);
}

