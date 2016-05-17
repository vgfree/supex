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
#include "comm_pipe.h"
#include "comm_list.h"
#include "mfptp_protocol/mfptp_parse.h"
#include "mfptp_protocol/mfptp_package.h"


#ifdef __cplusplus
extern "C" {
#endif

#define	EPOLL_SIZE		10000
#define LISTEN_SIZE		10	/* 允许监听fd的最大个数 */
#define TIMEOUTED		5000	/* 以毫秒(ms)为单位 1s = 1000ms*/
#define IPADDR_MAXSIZE		128
#define	CACHE_SIZE		1024
#define QUEUE_CAPACITY		50000
#define COMM_READ_MIOU		1024	/* 读取数据大小[getsockopt RCVBUF] */
#define COMM_WRITE_MIOU		1024	/* 写入数据大小[getsockopt SNDBUF] */
#define COMM_FRAMES		13

struct comm_context ;
struct portinfo	;
/* 回调函数的原型 */
typedef void (*CommCB)(struct comm_context* commctx, struct portinfo *portinfo, void* usr);

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
struct portinfo {
	int		fd;			/* 套接字描述符 */
	int		type;			/* 套接字的类型 */
	enum {
		FD_INIT = 0x01,
		FD_READ,
		FD_WRITE,
		FD_CLOSE
	}		stat;			/* 套接字的状态 */
	uint16_t	port;			/* 本地端口 */
	char		addr[IPADDR_MAXSIZE];	/* 本地IP地址 */
};

/* 此结构体保存发送接收时没成功处理完的fd */
struct  remainfd{
	int	wfda[EPOLL_SIZE/4];		/* 保存出现意外没有及时发送消息的fd */
	int	rfda[EPOLL_SIZE/4];		/* 保存出现意外没有及时接收消息的fd */
	int	wcnt;				/* 没有及时发送消息的fd的计数 */
	int	rcnt;				/* 没有及时接收消息的fd的计数 */
};

/* 内部结构体, 外部无需关心 */
struct comm_data {
	//struct comm_queue	recv_queue;	/* 存放接收并已经解析完毕的数据 */
	struct comm_queue	send_queue;	/* 存放用户传递但并未打包的数据 */
	struct comm_cache	recv_buff;	/* 存放接收但并未解析的数据 */
	struct comm_cache	send_buff;	/* 存放需要发送并已经打包的数据 */
	struct comm_lock	sendlock;	/* send_queue的锁 */
	struct comm_context*	commctx;	/* 通信上下文的结构体 */
	struct cbinfo		finishedcb;	/* 此描述符监听事件发生时相应的回调函数信息 */
	struct portinfo		portinfo;	/* 端口的相关信息 */
	struct mfptp_parser	parser;		/* 解析器 */
	struct mfptp_packager	packager;	/* 打包器 */
	int			parsepct;	/* 解析数据百分比[根据此值来决定什么时候调用解析函数] */
	int			packpct;	/* 打包数据百分比[根据此值来决定什么时候调用打包函数]*/
};

/* 监听描述符的相关信息 */
struct listenfd {
	int		counter;			/* 监听fd的计数器 */
	int		fd[LISTEN_SIZE];		/* 监听的每个fd */
	struct cbinfo	finishedcb[LISTEN_SIZE];	/* 每个fd对应的回调函数 */
	struct portinfo portinfo[LISTEN_SIZE];		/* 每个fd对应的端口信息 */
	struct comm_context* commctx;
};

/* 通信模块的上下文环境结构体 */
struct comm_context {
	pthread_t		ptid;			/* 新线程的pid */
	intptr_t		data[EPOLL_SIZE];	/* 保存接收发送的相关数据 */
	struct cbinfo		timeoutcb;		/* 超时回调函数的相关信息 */
	struct remainfd		remainfd;		/* 遗留下来没有及时处理的fd */
	struct listenfd		listenfd;		/* 所有监听描述符的信息 */
	struct comm_queue	recv_queue;		/* 存放已经接收到并解析好的数据 */
	struct comm_pipe	commpipe;		/* 关于管道的相关信息 */
	struct comm_lock	recvlock;		/* 用来同步接收队列的锁*/
	struct comm_lock	statlock;		/* 用来同步stat的状态 */
	struct comm_epoll	commepoll;		/* epoll监听事件的相关信息 */
	struct comm_list	head;			/* 存放send */
	enum {
		COMM_STAT_NONE,
		COMM_STAT_INIT,
		COMM_STAT_RUN,
		COMM_STAT_STOP
	}		stat;				/* 线程的状态 */
};

/* 数据包的设置 */
struct comm_package {
	int dsize;					/* 数据总大小 */
	int frames;					/* 总帧数 */
	int packages;					/* 总包数 */
	int frame_size[COMM_FRAMES];			/* 每个帧数据大小 */
	int frame_offset[COMM_FRAMES];			/* 每个帧的偏移 */
	int frames_of_package[COMM_FRAMES];		/* 每个包中帧的多少 */
};

/* 发送接收数据的结构体 */
struct comm_message {
	int			fd;			/* 消息对应的描述符 */
	int			config;			/* 消息的加密压缩格式 */
	int			socket_type;		/* 消息套接字的类型 */
	char*			content;		/* 消息的内容首地址 */
	struct comm_package	package;		/* 消息包的设置 */
	struct comm_list	list;			/* 链表节点 */
};

struct comm_data*  commdata_init(struct comm_context* commctx, struct portinfo* portinfo,  struct cbinfo*  finishedcb);

void commdata_destroy(struct comm_data *commdata);

struct comm_message* new_commmsg(int size);

void copy_commmsg(struct comm_message* destmsg, const struct comm_message* srcmsg);

void free_commmsg(void* arg);

bool get_portinfo( struct portinfo* portinfo, int fd, int type, int status);

void listenfd_init(struct listenfd *listenfd, struct comm_context *commctx);

void listenfd_destroy(struct listenfd *listenfd);

bool add_listenfd(struct listenfd *listenfd, struct cbinfo *finishedcb, struct portinfo *portinfo, int fd);

bool del_listenfd(struct listenfd *listenfd, int fdidx);

int search_listenfd(struct listenfd *listenfd, int fd);

#ifdef __cplusplus
	}
#endif

#endif /* ifndef __COMM_STRUCTURE_H__ */
