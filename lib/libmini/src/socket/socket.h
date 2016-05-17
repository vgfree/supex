//
//  socket.h
//
//
//  Created by 周凯 on 15/9/7.
//
//

#ifndef __libmini__socket__
#define __libmini__socket__

#include "socket_base.h"

__BEGIN_DECLS

typedef void (*SetSocketCB)(int, void *);
/**
 * 建立一个侦听服务器套接字
 * @param host 主机名，可以是ip地址或域名
 * @param serv 服务名，可以是端口或协议名次，如ftp
 * @param cb 创建套接字后的回调
 * @param usr 回调数据
 * @return 套接字描述符 -1失败
 */
int TcpListen(const char *host, const char *serv, SetSocketCB cb, void *usr);

/**
 * 建立一个客户端套接字
 * @param host 主机名，可以是ip地址或域名
 * @param serv 服务名，可以是端口或协议名次，如ftp
 * @param cb 创建套接字后的回调
 * @param usr 回调数据
 * @param timeout < 0 阻塞连接 ＝ 0 非阻塞连接 > 0超时连接
 * @return 套接字描述符 -1失败
 */
int TcpConnect(const char *host, const char *serv, SetSocketCB cb, void *usr, long timeout);

__END_DECLS
#endif	/* defined(__libmini__socket__) */

