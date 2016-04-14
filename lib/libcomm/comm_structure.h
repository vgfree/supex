/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_STRUCTURE_H__
#define __COMM_STRUCTURE_H__

#include "comm_utils.h"
#include "comm_cache.h"
#include "comm_queue.h"
#include "comm_tcp.h"
#include "comm_epoll.h"
#include "comm_lock.h"


#ifdef __cplusplus
extern "C" {
#endif

#define	EPOLL_SIZE		10000
#define TIMEOUTED		5000
#define IPADDR_MAXSIZE		128
#define	CACHE_SIZE		1024
#define QUEUE_CAPACITY		50000
#define COMM_READ_MIOU		1024	/* 读取数据大小[getsockopt RCVBUF] */
#define COMM_WRITE_MIOU		1024	/* 写入数据大小[getsockopt SNDBUF] */

struct comm_context ;

/* 回调函数的原型: fd参数:对于超时和线程回调函数无用，主要用于finished回调函数 */
typedef void (*CommCB)(struct comm_context* commctx, int fd, int status, void* usr);

/* 套接字的类型 */
enum {
	COMM_CONNECT = 0x01,
	COMM_BIND,
	COMM_ACCEPT
};

/* 回调函数的相关信息 */
struct cbinfo {
	int			timeout;
	CommCB			callback;	/* 相关的回调函数 */
	void*			usr;		/* 用户的参数 */
};

/* 端口的相关信息 */
struct portinfo{
	int		fd;			/* 套接字描述符 */
	int		type;			/* 套接字的类型 */
	uint16_t	port;			/* 本地端口 */
	char		addr[IPADDR_MAXSIZE];	/* 本地IP地址 */
};

/* 内部结构体, 外部无需关心 */
struct comm_data{
	struct comm_queue	recv_queue;	/* 存放接收并已经解析完毕的数据 */
	struct comm_queue	send_queue;	/* 存放用户传递但并未打包的数据 */
	struct comm_cache	recv_buff;	/* 存放接收但并未解析的数据 */
	struct comm_cache	send_buff;	/* 存放需要发送并已经打包的数据 */
	struct comm_context*	commctx;	/* 通信上下文的结构体 */
	struct cbinfo		finishedcb;	/* 此描述符监听事件发生时相应的回调函数信息 */
	struct portinfo		portinfo;	/* 端口的相关信息 */
	int			parsepct;	/* 解析数据百分比[根据此值来决定什么时候调用解析函数] */
	int			packpct;	/* 打包数据百分比[根据此值来决定什么时候调用打包函数]*/
};
/* 通信模块的上下文环境结构体 */
struct comm_context {
	int			epfd;			/* IO复用句柄 */
	int			listenfd;		/* 如果类型为COMM_BIND此变量会被赋值 */
	intptr_t		data[EPOLL_SIZE];	/* 保存接收发送的相关数据 */
	long			watchcnt;		/* 正在监听的fd的个数 */
	pthread_t		ptid;			/* 新线程的pid */
	struct cbinfo		timeoutcb;		/* 超时回调函数的相关信息 */
	struct comm_queue	recv_queue;		/* 存放已经接收到并解析好的数据 */
	struct comm_lock	commlock;		/* 用来同步stat的状态 */
	enum {
		COMM_STAT_NONE,
		COMM_STAT_INIT,
		COMM_STAT_RUN,
		COMM_STAT_STOP
	}		stat;			/* 线程的状态 */
};

/* 发送接收数据的结构体 */
struct comm_message {
	int	fd;					/* 消息对应的描述符 */
	int	encrypt;				/* 消息是否加密 */
	int	compress;				/* 消息是否压缩 */
	int	size;					/* 消息的大小 */
	char*	content;				/* 消息的内容首地址 */
};

struct comm_data*  commdata_init(struct comm_context* commctx, struct portinfo* portinfo,  struct cbinfo*  finishedcb);

void commdata_destroy(struct comm_data *commdata);

struct comm_message* new_commmsg(int size);

void copy_commmsg(struct comm_message* destmsg, const struct comm_message* srcmsg);

void free_commmsg(struct comm_message* message);



#ifdef __cplusplus
	}
#endif

#endif /* ifndef __COMM_STRUCTURE_H__ */
