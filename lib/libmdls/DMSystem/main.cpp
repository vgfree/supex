/*************************************************************************
 *        > File Name: main.cpp
 *        > Author: ma6174
 *        > Mail: ma6174@163.com
 *        > Created Time: Tue 12 Jan 2016 04:04:44 PM CST
 ************************************************************************/

#include <iostream>
#include <netinet/in.h>		// for sockaddr_in
#include <sys/types.h>		// for socket
#include <sys/socket.h>		// for socket
#include <string>
#include <arpa/inet.h>

#include "PlatformHeader.h"

#include "QTRTPFile.h"

using namespace std;

#define BUFFER_SIZE     1024
#define DATA_LENGTH     512

#define IP_ADDRESS      "192.168.11.222"
#define PORT            560
int main()
{
	std::cout << "********** DMSystem start **********\n\n";
	QTRTPFile::Initialize();

	int                     ret = 0;
	int                     svrsocket;
	struct sockaddr_in      svraddr;

	bzero(&svraddr, sizeof(svraddr));

	svraddr.sin_family = AF_INET;
	svraddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
	svraddr.sin_port = htons(PORT);

	svrsocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (svrsocket < 0) {
		qtss_printf("DMSystem create socket error\n");
		return -1;
	}

	int opt = 1;
	setsockopt(svrsocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	ret = bind(svrsocket, (struct sockaddr *)&svraddr, sizeof(svraddr));

	if (ret != 0) {
		qtss_printf("DMSystem bind error.\n");
		return -1;
	}

	ret = listen(svrsocket, 10);

	if (ret != 0) {
		qtss_printf("DMSystem listen error.\n");
		return -1;
	}

	qtss_printf("DMSystem listen in ip:%d, port:%d\n", svraddr.sin_addr.s_addr, ntohs(svraddr.sin_port));

	while (1) {
		struct sockaddr_in      client_addr;
		socklen_t               length = sizeof(client_addr);
		// 接受连接
		int clientsocket = accept(svrsocket, (struct sockaddr *)&client_addr, &length);

		if (clientsocket < 0) {
			qtss_printf("DMSystem accept error.\n");
			// system("pause");
			break;
		}

		qtss_printf("DMSystem accept client ip:%d, socket:%d connect.\n", client_addr.sin_addr.s_addr, clientsocket);

		char buffer[BUFFER_SIZE];
		bzero(buffer, BUFFER_SIZE);

		// send test
		send(clientsocket, buffer, 1024, 0);
		length = recv(clientsocket, buffer, BUFFER_SIZE, 0);

		if (length < 0) {
			qtss_printf("DMSystem recv data failed.\n");
			break;
		}

		char data[DATA_LENGTH];
		bzero(data, DATA_LENGTH);
		strncpy(data, buffer, strlen(buffer) > DATA_LENGTH ? DATA_LENGTH : strlen(buffer));
		qtss_printf("DMSystem recv data:%s \n\n", data);

		close(clientsocket);
	}

	// application exit
	close(svrsocket);
	// 调用接口
	std::cout << "********** DMSystem exit **********\n\n";
	return 0;
}

