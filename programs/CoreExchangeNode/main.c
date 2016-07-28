#include <signal.h>
#include <stdlib.h>

#include "config_reader.h"
#include "communication.h"
#include "daemon.h"
#include "fd_manager.h"
#include "message_dispatch.h"
#include "notify.h"
#include "uid_map.h"
#include "gid_map.h"
#include "libmini.h"

#define CONFIG                          "core_exchange_node.conf"
#define LISTEN_MESSAGEGATEWAY_HOST      "ListenMessageGatewayHost"
#define LISTEN_MESSAGEGATEWAY_PORT      "ListenMessageGatewayPort"
#define LISTEN_CLIENT_HOST              "ListenClientHost"
#define LISTEN_CLIENT_PORT              "ListenClientPort"
#define SERVER_FILE                     "CoreExchangeNode.pid"
#define PACKAGE_SIZE                    "PackageSize"
#define MODULE_NAME                     "CoreExchangeNode"
#define CONNECT_MESSAGEGATEWAY_HOST     "ConnectMessageGatewayHost"
#define CONNECT_MESSAGEGATEWAY_PORT     "ConnectMessageGatewayPort"
#define CONNECT_SETTINGSERVER_HOST      "ConnectSettingServerHost"
#define CONNECT_SETTINGSERVER_PORT      "ConnectSettingServerPort"
#define CONNECT_LOGINSERVER_HOST        "ConnectLoginServerHost"
#define CONNECT_LOGINSERVER_PORT        "ConnectLoginServerPort"

static int work_init(void)
{
	/*init log*/
	SLogOpen(MODULE_NAME ".log", SLogIntegerToLevel(1));

	/*get config*/
	struct config_reader    *config = init_config_reader(CONFIG);
	char                    *clientHost = get_config_name(config, LISTEN_CLIENT_HOST);
	char                    *clientPort = get_config_name(config, LISTEN_CLIENT_PORT);
	x_printf(D, "ListenClientHost = %s, ListenClientPort = %s", clientHost, clientPort);
	
	char    *MGcliHost = get_config_name(config, CONNECT_MESSAGEGATEWAY_HOST);
	char    *MGcliPort = get_config_name(config, CONNECT_MESSAGEGATEWAY_PORT);
	x_printf(D, "MGcliHost:%s, MGcliPort:%s.", MGcliHost, MGcliPort);

	char    *settingServerHost = get_config_name(config, CONNECT_SETTINGSERVER_HOST);
	char    *settingServerPort = get_config_name(config, CONNECT_SETTINGSERVER_PORT);
	x_printf(D, "SettingHost:%s, SettingPort:%s.", settingServerHost, settingServerPort);
	
	char    *loginServerHost = get_config_name(config, CONNECT_LOGINSERVER_HOST);
	char    *loginServerPort = get_config_name(config, CONNECT_LOGINSERVER_PORT);
	x_printf(D, "loginServerHost:%s, loginServerPort:%s.", loginServerHost, loginServerPort);


	/*space init*/
	fdman_list_init();
	fdman_array_init();

	/*io init*/
	struct comm_context *commctx = comm_ctx_create(EPOLL_SIZE);
	assert(commctx);

	/*init client event*/
	struct cbinfo clientCB = {};
	clientCB.callback = client_event_notify;
	int retval = comm_socket(commctx, clientHost, clientPort, &clientCB, COMM_BIND);
	if (retval == -1) {
		x_printf(E, "can't bind client socket, ip:%s, port:%s.", clientHost, clientPort);
		return -1;
	}

	/*init messageGetway event*/
	int msg_fd = 0;
	if (MGcliHost) {
		struct cbinfo MGCB = {};
		MGCB.callback = message_gateway_event_notify;
		msg_fd = comm_socket(commctx, MGcliHost, MGcliPort, &MGCB, COMM_CONNECT | CONNECT_ANYWAY);
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
		struct cbinfo settingServerCB = {};
		settingServerCB.callback = setting_server_event_notify;
		set_fd = comm_socket(commctx, settingServerHost, settingServerPort, &settingServerCB, COMM_CONNECT | CONNECT_ANYWAY);
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
		struct cbinfo loginServerCB = {};
		loginServerCB.callback = login_server_event_notify;
		gin_fd = comm_socket(commctx, loginServerHost, loginServerPort, &loginServerCB, COMM_CONNECT | CONNECT_ANYWAY);
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

	/*init maps*/
	init_uid_map();
	init_gid_map();
	/*fill serv*/
	g_serv_info.commctx = commctx;
	strcpy(g_serv_info.host, clientHost);
	g_serv_info.port = atoi(clientPort);
	g_serv_info.message_gateway_fd = msg_fd;
	g_serv_info.setting_server_fd = set_fd;
	g_serv_info.login_server_fd = gin_fd;

	/*init over*/
	destroy_config_reader(config);
	return 0;
}

int main(void)
{
	signal(SIGPIPE, SIG_IGN);

	if (daemon_init(SERVER_FILE) == 1) {
		printf("ERROR:Server is running.");
		return -1;
	}

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
	daemon_exit(SERVER_FILE);
	return 0;
}

