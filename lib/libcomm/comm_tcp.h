/*********************************************************************************************/
/************************	Created by 许莉 on 16/03/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_TCP_H__
#define __COMM_TCP_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "comm_utils.h"

#ifdef __cplusplus
extern "C" {
#endif



/* 通过套接字描述符获取本地的IP地址:本地字节序 */
int get_address(int fd, char *paddr, size_t plen);

/* 通过套接字描述符获取本地的端口:本地字节序 */
uint16_t get_port(int fd);

/* 绑定监听一个指定地址端口号 */
int socket_listen(const char* host, const char* server);

/* 连接一个指定的地址端口号 */
int socket_connect(const char* host, const char* server);


#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_TCP_H__ */

