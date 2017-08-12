#include <signal.h>
#include <stdlib.h>

#include "iniparser.h"
#include "comm_api.h"
#include "fd_manager.h"
#include "message_dispatch.h"
#include "notify.h"
#include "uid_map.h"
#include "gid_map.h"
#include "libmini.h"
#include "comm_def.h"


static int work_init(void)
{
	/*init log*/
	SLogOpen(MODULE_NAME ".log", SLogIntegerToLevel(1));

	/*get config*/
	dictionary    *config = iniparser_load(CONFIG);
	char                    *clientHost = iniparser_getstring(config, LISTEN_CLIENT_HOST, NULL);
	char                    *clientPort = iniparser_getstring(config, LISTEN_CLIENT_PORT, NULL);
	x_printf(D, "ListenClientHost = %s, ListenClientPort = %s", clientHost, clientPort);
	
	char    *MGcliHost = iniparser_getstring(config, CONNECT_MESSAGEGATEWAY_HOST, NULL);
	char    *MGcliPort = iniparser_getstring(config, CONNECT_MESSAGEGATEWAY_PORT, NULL);
	x_printf(D, "MGcliHost:%s, MGcliPort:%s.", MGcliHost, MGcliPort);

	char    *settingServerHost = iniparser_getstring(config, CONNECT_SETTINGSERVER_HOST, NULL);
	char    *settingServerPort = iniparser_getstring(config, CONNECT_SETTINGSERVER_PORT, NULL);
	x_printf(D, "SettingHost:%s, SettingPort:%s.", settingServerHost, settingServerPort);
	
	char    *loginServerHost = iniparser_getstring(config, CONNECT_LOGINSERVER_HOST, NULL);
	char    *loginServerPort = iniparser_getstring(config, CONNECT_LOGINSERVER_PORT, NULL);
	x_printf(D, "loginServerHost:%s, loginServerPort:%s.", loginServerHost, loginServerPort);


	/*space init*/
	fdman_list_init();
	fdman_array_init();

	/*io init*/
	struct comm_context *commctx = commapi_ctx_create();
	assert(commctx);
	g_serv_info.commctx = commctx;

	/*init client event*/
	struct comm_cbinfo clientCB = {};
	clientCB.fcb = client_event_notify;
	int retval = commapi_socket(commctx, clientHost, clientPort, &clientCB, COMM_BIND);
	if (retval == -1) {
		x_printf(E, "can't bind client socket, ip:%s, port:%s.", clientHost, clientPort);
		return -1;
	}

	/*init messageGetway event*/
	int msg_fd = 0;
	if (MGcliHost) {
		struct comm_cbinfo MGCB = {};
		MGCB.fcb = message_gateway_event_notify;
		g_serv_info.message_gateway_fd = msg_fd = commapi_socket(commctx, MGcliHost, MGcliPort, &MGCB, COMM_CONNECT);
		if (msg_fd <= 0) {
			x_printf(E, "connect message gateway failed.");
			return -1;
		}
		
		struct fd_descriptor des = {};
		des.status = 1;
		des.obj = MESSAGE_GATEWAY;
		make_uuid(des.uuid);
		fdman_array_fill_fd(msg_fd, &des);

		struct fd_node node = {};
		node.fd = msg_fd;
		node.status = 1;
		fdman_list_push_back(MESSAGE_GATEWAY, &node);
	}

	/*init setting event*/
	int set_fd = 0;
	if (settingServerHost) {
		struct comm_cbinfo settingServerCB = {};
		settingServerCB.fcb = setting_server_event_notify;
		g_serv_info.setting_server_fd = set_fd = commapi_socket(commctx, settingServerHost, settingServerPort, &settingServerCB, COMM_CONNECT);
		if (set_fd <= 0) {
			x_printf(E, "connect settingServer failed.");
			return -1;
		}

		struct fd_descriptor des = {};
		des.status = 1;
		des.obj = SETTING_SERVER;
		make_uuid(des.uuid);
		fdman_array_fill_fd(set_fd, &des);

		struct fd_node node = {};
		node.fd = set_fd;
		node.status = 1;
		fdman_list_push_back(SETTING_SERVER, &node);
	}

	/*init login event*/
	int gin_fd = 0;
	if (loginServerHost) {
		struct comm_cbinfo loginServerCB = {};
		loginServerCB.fcb = login_server_event_notify;
		g_serv_info.login_server_fd = gin_fd = commapi_socket(commctx, loginServerHost, loginServerPort, &loginServerCB, COMM_CONNECT);
		if (gin_fd <= 0) {
			x_printf(E, "connect loginServer failed.");
			return -1;
		}
		
		struct fd_descriptor des = {};
		des.status = 1;
		des.obj = LOGIN_SERVER;
		make_uuid(des.uuid);
		fdman_array_fill_fd(gin_fd, &des);

		struct fd_node node = {};
		node.fd = gin_fd;
		node.status = 1;
		fdman_list_push_back(LOGIN_SERVER, &node);
	}

	kv_init();
	/*init maps*/
	init_uid_map();
	init_gid_map();
	/*fill serv*/
	strcpy(g_serv_info.host, clientHost);
	g_serv_info.port = atoi(clientPort);

	/*init over*/
	iniparser_freedict(config);
	return 0;
}

int main(void)
{
	if (work_init() == -1) {
		printf("ERROR:Server init failed.");
		return -1;
	}

	while (1) {
		message_dispatch();
	}

	fdman_array_destroy();
	fdman_list_destroy();
	destroy_uid_map();
	destroy_gid_map();
	kv_destroy();
	return 0;
}

