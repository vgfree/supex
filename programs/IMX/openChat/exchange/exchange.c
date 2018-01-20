#include <signal.h>
#include <stdlib.h>

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
	dictionary    *config = iniparser_load(CONFIG);
	char                    *clientHost = iniparser_getstring(config, LISTEN_CLIENT_HOST, NULL);
	char                    *clientPort = iniparser_getstring(config, LISTEN_CLIENT_PORT, NULL);
	x_printf(D, "ListenClientHost = %s, ListenClientPort = %s", clientHost, clientPort);
	
	char    *messageHost = iniparser_getstring(config, CONNECT_MESSAGEGATEWAY_HOST, NULL);
	char    *messagePort = iniparser_getstring(config, CONNECT_MESSAGEGATEWAY_PORT, NULL);
	x_printf(D, "messageHost:%s, messagePort:%s.", messageHost, messagePort);

	char    *controlHost = iniparser_getstring(config, CONNECT_SETTINGSERVER_HOST, NULL);
	char    *controlPort = iniparser_getstring(config, CONNECT_SETTINGSERVER_PORT, NULL);
	x_printf(D, "SettingHost:%s, SettingPort:%s.", controlHost, controlPort);
	
	char    *loginHost = iniparser_getstring(config, CONNECT_LOGINSERVER_HOST, NULL);
	char    *loginPort = iniparser_getstring(config, CONNECT_LOGINSERVER_PORT, NULL);
	x_printf(D, "loginHost:%s, loginPort:%s.", loginHost, loginPort);


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

	/*init client event*/
	struct comm_cbinfo clientCB = {};
	clientCB.fcb = exc_event_notify_from_client;
	int retval = commapi_socket(commctx, clientHost, clientPort, &clientCB, COMM_BIND);
	if (retval == -1) {
		x_printf(E, "can't bind client socket, ip:%s, port:%s.", clientHost, clientPort);
		return -1;
	}

	/*init messageGetway event*/
	int msg_fd = 0;
	if (messageHost) {
		struct comm_cbinfo MGCB = {};
		MGCB.fcb = exc_event_notify_from_message;
		g_serv_info.message_gateway_fd = msg_fd = commapi_socket(commctx, messageHost, messagePort, &MGCB, COMM_CONNECT);
		if (msg_fd <= 0) {
			x_printf(E, "connect message gateway failed.");
			return -1;
		}

		struct fd_info des = {};
		des.status = 1;
		des.type = MESSAGE_ROUTER;
		make_uuid(des.uuid);
		fdman_slot_set(msg_fd, &des);

		struct fd_node node = {};
		node.fd = msg_fd;
		node.status = 1;
		fdman_list_add(MESSAGE_ROUTER, &node);
	}

	/*init controlGateway event*/
	int ctl_fd = 0;
	if (controlHost) {
		struct comm_cbinfo controlCB = {};
		controlCB.fcb = exc_event_notify_from_control;
		g_serv_info.control_gateway_fd = ctl_fd = commapi_socket(commctx, controlHost, controlPort, &controlCB, COMM_CONNECT);
		if (ctl_fd <= 0) {
			x_printf(E, "connect control gateway failed.");
			return -1;
		}

		struct fd_info des = {};
		des.status = 1;
		des.type = CONTROL_ROUTER;
		make_uuid(des.uuid);
		fdman_slot_set(ctl_fd, &des);

		struct fd_node node = {};
		node.fd = ctl_fd;
		node.status = 1;
		fdman_list_add(CONTROL_ROUTER, &node);
	}

	/*init loginGateway event*/
	int gin_fd = 0;
	if (loginHost) {
		struct comm_cbinfo loginCB = {};
		loginCB.fcb = exc_event_notify_from_login;
		g_serv_info.login_gateway_fd = gin_fd = commapi_socket(commctx, loginHost, loginPort, &loginCB, COMM_CONNECT);
		if (gin_fd <= 0) {
			x_printf(E, "connect login gateway failed.");
			return -1;
		}
		
		struct fd_info des = {};
		des.status = 1;
		des.type = LOGIN_ROUTER;
		make_uuid(des.uuid);
		fdman_slot_set(gin_fd, &des);

		struct fd_node node = {};
		node.fd = gin_fd;
		node.status = 1;
		fdman_list_add(LOGIN_ROUTER, &node);
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
	return;
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
