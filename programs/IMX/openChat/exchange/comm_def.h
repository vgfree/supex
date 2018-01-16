#pragma once

#define FD_CAPACITY             1024 * 100
#define MAX_ONE_CID_HAVE_GID	100
#define MAX_ONE_GID_HAVE_CID	100
#define MAX_CID_SIZE		128
#define MAX_UID_SIZE		128
#define MAX_GID_SIZE		128


#define CONFIG                          "exchange.conf"

#define LISTEN_MESSAGEGATEWAY_HOST      ":ListenMessageGatewayHost"
#define LISTEN_MESSAGEGATEWAY_PORT      ":ListenMessageGatewayPort"
#define LISTEN_CLIENT_HOST              ":ListenClientHost"
#define LISTEN_CLIENT_PORT              ":ListenClientPort"
#define CONNECT_MESSAGEGATEWAY_HOST     ":ConnectMessageGatewayHost"
#define CONNECT_MESSAGEGATEWAY_PORT     ":ConnectMessageGatewayPort"
#define CONNECT_SETTINGSERVER_HOST      ":ConnectSettingServerHost"
#define CONNECT_SETTINGSERVER_PORT      ":ConnectSettingServerPort"
#define CONNECT_LOGINSERVER_HOST        ":ConnectLoginServerHost"
#define CONNECT_LOGINSERVER_PORT        ":ConnectLoginServerPort"
