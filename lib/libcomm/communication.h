/*********************************************************************************************/
/************************	Created by 许莉 on 16/02/25.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "comm_utils.h"
#include "comm_cache.h"
#include "comm_queue.h"
#include "comm_tcp.h"

#define	EPOLL_SIZE		10000
#define TIMEOUTED		5000
#define IPADDR_MAXSIZE		128
#define	CACHE_SIZE		1024
#define QUEUE_CAPACITY		50000

//套接字的类型
enum {
	COMM_CONNECT = 0x01,
	COMM_BIND,
	COMM_ACCEPT
};

//回调函数的原型: fd参数:对于超时和线程回调函数无用，主要用于finished回调函数
typedef void (*CommCB)(struct comm* commctx, int fd, void* usr);

//回调函数的相关信息
struct cbinfo{
	int			timeout;
	CommCB			callback;	//相关的回调函数
	void*			usr;		//用户的参数
};

//端口的相关信息
struct portinfo {
	int		fd;			//套接字描述符
	int		type;			//套接字的类型
	uint16_t	port;			//本地端口
	char		addr[IPADDR_MAXSIZE];	//本地IP地址
};

//内部结构体, 外部无需关心
struct comm_data{
	struct comm_queue	recv_queue;	//存放接收并已经解析完毕的数据
	struct comm_queue	send_queue;	//存放已经打包好需要发送的数据
	struct comm_cache	recv_buff;	//存放接收但并未解析的数据
	struct comm_cache	send_buff;	//存放需要发送但并未打包的数据 
	struct comm*		commctx;	//通信上下文的结构体
	struct cbinfo		finishedcb;	//此描述符监听事件发生时相应的回调函数信息
	struct portinfo		portinfo;	//端口的相关信息
};

struct comm {
	int		epfd;			//IO复用句柄
	int		listenfd;		//如果类型为COMM_BIND此变量会被赋值
        struct comm_data* data[EPOLL_SIZE];	//保存接收发送的相关数据
	long		watchcnt;		//正在监听的fd的个数
	pthread_t	ptid;			//新线程的pid
	struct cbinfo	timeoutcb;		//超时回调函数的相关信息
	struct epoll_event *events;		//监听的所有fd
	enum {
		COMM_STAT_NONE,
		COMM_STAT_INIT,
		COMM_STAT_RUN,
		COMM_STAT_STOP,
	}		stat;			//线程的状态
};

//创建一个通信上下文的结构体 epollsize:
struct comm* comm_ctx_create(int epollsize);

//销毁一个通信上下文的结构体
void comm_ctx_destroy(struct comm* commctx);

//bind或者connect某个指定的地址端口
int comm_socket(struct comm *commctx, char *host, char *server, struct cbinfo finishedcb, int type);

//发送数据
int comm_send(struct comm *commctx, int fd, const char *buff, int size);

//接收数据
int comm_recv(struct comm *commctx, int fd, char *buff, int size);

//关闭套接字
void comm_close(struct comm* commctx, int fd);

//设置epoll_wait的超时时间
void comm_settimeout(struct comm *commctx, int timeout, CommCB callback, void *usr);

#endif
