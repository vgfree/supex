#include "config_reader.h"
#include "communication.h"
#include "daemon.h"
#include "fd_manager.h"
#include "gid_map.h"
#include "loger.h"
#include "message_dispatch.h"
#include "notify.h"
#include "uid_map.h"

#include <signal.h>
#include <stdlib.h>

#define CONFIG                          "core_exchange_node.conf"
#define LISTEN_MESSAGEGATEWAY_IP        "ListenMessageGatewayIP"
#define LISTEN_MESSAGEGATEWAY_PORT      "ListenMessageGatewayPort"
#define LISTEN_CLIENT_IP                "ListenClientIP"
#define LISTEN_CLIENT_PORT              "ListenClientPort"
#define SERVER_FILE                     "CoreExchangeNode.pid"
#define PACKAGE_SIZE                    "PackageSize"
#define MODULE_NAME                     "CoreExchangeNode"
#define CONNECT_MESSAGEGATEWAY_IP       "ConnectMessageGatewayIP"
#define CONNECT_MESSAGEGATEWAY_PORT     "ConnectMessageGatewayPort"
#define CONNECT_SETTINGSERVER_IP        "ConnectSettingServerIP"
#define CONNECT_SETTINGSERVER_PORT      "ConnectSettingServerPort"
#define CONNECT_LOGINSERVER_IP          "ConnectLoginServerIP"
#define CONNECT_LOGINSERVER_PORT        "ConnectLoginServerPort"

struct CSLog *g_imlog = NULL;

static int init()
{
	g_imlog = CSLog_create(MODULE_NAME, WATCH_DELAY_TIME);
	struct config_reader *config =
		init_config_reader(CONFIG);
	char    *clientIP = get_config_name(config, LISTEN_CLIENT_IP);
	char    *clientPort = get_config_name(config, LISTEN_CLIENT_PORT);
	//  char *MGsrvIP = get_config_name(config, LISTEN_MESSAGEGATEWAY_IP);
	//  char *MGsrvPort = get_config_name(config, LISTEN_MESSAGEGATEWAY_PORT);
	//  char *package_sz = get_config_name(config, PACKAGE_SIZE);
	log("ListenClientIp = %s, ListenClientPort = %s", clientIP, clientPort);
	//  log("MGsrvIp = %s, MGsrvPort = %s", MGsrvIP, MGsrvPort);
	list_init();
	array_init();
	struct comm_context *commctx = NULL;
	commctx = comm_ctx_create(EPOLL_SIZE);

	if (unlikely(!commctx)) {
		return -1;
	}

	struct cbinfo clientCB = {};
	clientCB.callback = client_event_notify;

	int retval = comm_socket(commctx, clientIP, clientPort, &clientCB, COMM_BIND);

	if (retval == -1) {
		error("can't bind client socket, ip:%s, port:%s.", clientIP, clientPort);
		return retval;
	}

	struct cbinfo MGCB = {};
	MGCB.callback = message_gateway_event_notify;

	/*  retval = comm_socket(commctx, MGsrvIP, MGsrvPort, &MGCB, COMM_BIND);
	 *   if (retval == -1) {
	 *    error("can't bind MG socket, ip:%s, port:%s.", MGsrvIP, MGsrvPort);
	 *   }*/
	strcpy(g_serv_info.ip, clientIP);
	g_serv_info.port = atoi(clientPort);
	//  g_serv_info.package_size = atoi(package_sz);
	g_serv_info.commctx = commctx;
	log("port:%d", g_serv_info.port);
	char    *MGcliIP = get_config_name(config, CONNECT_MESSAGEGATEWAY_IP);
	char    *MGcliPort = get_config_name(config, CONNECT_MESSAGEGATEWAY_PORT);
	log("MGcliIP:%s, MGcliPort:%s.", MGcliIP, MGcliPort);

	if (MGcliIP) {
		int fd =
			comm_socket(commctx, MGcliIP, MGcliPort, &MGCB, COMM_CONNECT | CONNECT_ANYWAY);

		if (fd > 0) {
			struct fd_descriptor des = {};
			des.status = 1;
			des.obj = MESSAGE_GATEWAY;
			array_fill_fd(fd, &des);

			struct fd_node node = {};
			node.fd = fd;
			g_serv_info.message_gateway_fd = fd;
			node.status = 1;
			list_push_back(MESSAGE_GATEWAY, &node);
		} else {
			error("connect message gateway failed.");
			return -1;
		}
	}

	char    *settingServerIp = get_config_name(config, CONNECT_SETTINGSERVER_IP);
	char    *settingServerPort = get_config_name(config, CONNECT_SETTINGSERVER_PORT);

	if (settingServerIp) {
		struct cbinfo settingServerCB = {};
		settingServerCB.callback = setting_server_event_notify;
		int fd =
			comm_socket(commctx, settingServerIp, settingServerPort,
				&settingServerCB, COMM_CONNECT | CONNECT_ANYWAY);
		log("setting server fd:%d.", fd);

		if (fd > 0) {
			struct fd_descriptor des = {};
			des.status = 1;
			des.obj = SETTING_SERVER;
			array_fill_fd(fd, &des);

			struct fd_node node = {};
			node.fd = fd;
			g_serv_info.setting_server_fd = fd;
			node.status = 1;
			list_push_back(SETTING_SERVER, &node);
		} else {
			error("connect settingServer failed.");
			return -1;
		}
	}

	char    *loginServerIp = get_config_name(config, CONNECT_LOGINSERVER_IP);
	char    *loginServerPort = get_config_name(config, CONNECT_LOGINSERVER_PORT);

	if (loginServerIp) {
		struct cbinfo loginServerCB = {};
		loginServerCB.callback = login_server_event_notify;
		int fd =
			comm_socket(commctx, loginServerIp, loginServerPort,
				&loginServerCB, COMM_CONNECT | CONNECT_ANYWAY);
		log("login server fd:%d.", fd);

		if (fd > 0) {
			struct fd_descriptor des = {};
			des.status = 1;
			des.obj = LOGIN_SERVER;
			array_fill_fd(fd, &des);

			struct fd_node node = {};
			node.fd = fd;
			g_serv_info.login_server_fd = fd;
			node.status = 1;
			list_push_back(LOGIN_SERVER, &node);
		} else {
			error("connect loginServer failed.");
			return -1;
		}
	}

	destroy_config_reader(config);
	init_uid_map();
	init_gid_map();
#ifdef _HKEY_
	hkey_init();
#endif
	return 0;
}

int main(int argc, char *argv[])
{
	signal(SIGPIPE, SIG_IGN);

	if (daemon_init(SERVER_FILE) == 1) {
		printf("Server is running.");
		return -1;
	}

	if (init() == -1) {
		error("server init failed.");
		return -1;
	}

	while (1) {
		message_dispatch();
	}

	array_destroy();
	list_destroy();
	destroy_uid_map();
	destroy_gid_map();
#ifdef _HKEY_
	hkey_destroy();
#endif
	CSLog_destroy(g_imlog);
	daemon_exit(SERVER_FILE);
	return 0;
}

