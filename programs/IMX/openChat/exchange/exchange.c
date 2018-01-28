#include <signal.h>
#include <stdlib.h>

#include "libkv.h"
#include "libmini.h"
#include "iniparser.h"
#include "comm_api.h"

#include "exc_comm_def.h"
#include "exc_sockfd_manager.h"
#include "exc_message_dispatch.h"
#include "exc_event_notify.h"
#include "exc_cid_map.h"
#include "exc_uid_map.h"
#include "exc_gid_map.h"

static int _do_init(void)
{
	/*get config*/
	dictionary      *config = iniparser_load(CONFIG);
	const char            *clientHost = iniparser_getstring(config, BIND_CLIENT_STREAM_HOST, NULL);
	const char            *clientPort = iniparser_getstring(config, BIND_CLIENT_STREAM_PORT, NULL);

	x_printf(D, "BindClientStreamHost:%s, BindClientStreamPort:%s", clientHost, clientPort);

	const char    *statusHost = iniparser_getstring(config, CONN_GATEWAY_STATUS_HOST, NULL);
	const char    *statusPort = iniparser_getstring(config, CONN_GATEWAY_STATUS_PORT, NULL);
	x_printf(D, "ConnGatewayStatusHost:%s, ConnGatewayStatusPort:%s.", statusHost, statusPort);

	const char    *streamHost = iniparser_getstring(config, CONN_GATEWAY_STREAM_HOST, NULL);
	const char    *streamPort = iniparser_getstring(config, CONN_GATEWAY_STREAM_PORT, NULL);
	x_printf(D, "ConnGatewayStreamHost:%s, ConnGatewayStreamPort:%s.", streamHost, streamPort);

	const char    *manageHost = iniparser_getstring(config, CONN_GATEWAY_MANAGE_HOST, NULL);
	const char    *managePort = iniparser_getstring(config, CONN_GATEWAY_MANAGE_PORT, NULL);
	x_printf(D, "ConnGatewayManageHost:%s, ConnGatewayManagePort:%s.", manageHost, managePort);

	kv_init();
	/*space init*/
	fdman_list_init();
	fdman_slot_init();
	/*init maps*/
	exc_uidmap_init();
	exc_cidmap_init();
	exc_gidmap_init();

	/*fill serv*/
	strcpy(g_serv_info.host, clientHost);
	g_serv_info.port = atoi(clientPort);

	/*io init*/
	struct comm_context *commctx = commapi_ctx_create();
	assert(commctx);
	g_serv_info.commctx = commctx;

	int                     sktfd = 0;
	struct comm_cbinfo      cbinfo = {};
	/*init streamGetway event*/
	cbinfo.fcb = exc_event_notify_from_stream;
	g_serv_info.stream_gateway_fd = sktfd = commapi_socket(commctx, streamHost, streamPort, &cbinfo, COMM_CONNECT);

	if (sktfd <= 0) {
		x_printf(E, "connect streamGateway %s:%s failed.", streamHost, streamPort);
		return -1;
	}

	/*init manageGateway event*/
	cbinfo.fcb = exc_event_notify_from_manage;
	g_serv_info.manage_gateway_fd = sktfd = commapi_socket(commctx, manageHost, managePort, &cbinfo, COMM_CONNECT);

	if (sktfd <= 0) {
		x_printf(E, "connect manageGateway %s:%s failed.", manageHost, managePort);
		return -1;
	}

	/*init statusGateway event*/
	cbinfo.fcb = exc_event_notify_from_status;
	g_serv_info.status_gateway_fd = sktfd = commapi_socket(commctx, statusHost, statusPort, &cbinfo, COMM_CONNECT);

	if (sktfd <= 0) {
		x_printf(E, "connect statusGateway %s:%s failed.", statusHost, statusPort);
		return -1;
	}

	/*init client event*/
	/*注意:所有gateway启动完毕后再启动exchange*/
	cbinfo.fcb = exc_event_notify_from_client;
	int retval = commapi_socket(commctx, clientHost, clientPort, &cbinfo, COMM_BIND);

	if (retval == -1) {
		x_printf(E, "can't bind client socket, ip:%s, port:%s.", clientHost, clientPort);
		return -1;
	}

	/*init over*/
	iniparser_freedict(config);
	return 0;
}

static void *_do_work(void *usr)
{
	while (1) {
		exc_message_dispatch();
	}

	return NULL;
}

static pthread_t tid1;
void exchange_work(void)
{
	if (_do_init() == -1) {
		printf("ERROR:Server init failed.");
		exit(-1);
	}

	/*work push*/
	int err = pthread_create(&tid1, NULL, _do_work, NULL);

	if (err != 0) {
		x_printf(E, "can't create exchange thread:%s.", strerror(err));
	}

	x_printf(I, "exchange work!\n");
}

void exchange_wait(void)
{
	/*over*/
	void *status = NULL;

	pthread_join(tid1, status);
}

void exchange_stop(void)
{
	x_printf(W, "exchange stop!\n");

	fdman_slot_free();
	fdman_list_free();
	exc_uidmap_free();
	exc_cidmap_free();
	exc_gidmap_free();
	kv_destroy();
}

