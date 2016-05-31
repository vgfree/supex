/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_STRUCTURE_H__
#define __COMM_STRUCTURE_H__

#include "comm_list.h"
#include "comm_tcp.h"
#include "comm_cache.h"
#include "comm_queue.h"
#include "comm_epoll.h"
#include "comm_lock.h"
#include "comm_pipe.h"
#include "mfptp_protocol/mfptp_parse.h"
#include "mfptp_protocol/mfptp_package.h"


#ifdef __cplusplus
extern "C" {
#endif

#define	EPOLL_SIZE		1000	/* 允许EPOLL能够监听的描述符的最大个数 */
#define LISTEN_SIZE		10	/* 允许监听fd的最大个数 */
#define COMM_FRAMES		13	/* 允许一个包最大的总帧数 */
#define	REMAINFD_INCREASE_SIZE	10	/* remainfd扩容时的增加幅度 */

/* 获取struct comm_message结构体中成员变量list的偏移大小 */
#define	COMMMSG_OFFSET		({ struct comm_message message;	\
				   get_member_offset(&message, &message.list); \
				})				


struct comm_event;
struct comm_context;
/********************************  以下为外部结构体 外部使用需要了解 ****************************************/

/* 回调函数的原型 */
typedef void (*CommCB)(struct comm_context* commctx, struct comm_tcp* commtcp, void* usr);

/* 回调函数的相关信息 */
struct cbinfo {
	int			timeout;
	CommCB			callback;	/* 相关的回调函数 */
	void*			usr;		/* 用户的参数 */
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


/* 通信模块的上下文环境结构体 */
struct comm_context {
	pthread_t		ptid;			/* 新线程的pid */
	struct comm_event*	commevent;		/* 事件驱动的相关信息 */
	struct comm_queue	recvqueue;		/* 存放已经接收到并解析好的数据 */
	struct comm_list	recvlist;		/* 当recvqueue已满时放入此链表中 */
	struct comm_pipe	commpipe;		/* 关于管道的相关信息 */
	struct comm_lock	recvlock;		/* 用来同步接收队列的锁*/
	struct comm_lock	statlock;		/* 用来同步stat的状态 */
	struct comm_epoll	commepoll;		/* epoll监听事件的相关信息 */
	enum {
		COMM_STAT_NONE,
		COMM_STAT_INIT,
		COMM_STAT_RUN,
		COMM_STAT_STOP
	}		stat;				/* 线程的状态 */
};


/********************************  以下为内部结构体 外部无需关心 ********************************************/

/* 残留的fd的类型 */
enum remainfd_type {
	REMAINFD_WRITE = 0x01,
	REMAINFD_READ,
	REMAINFD_PARSE,
	REMAINFD_PACK
};

/* 此结构体保存发送接收时没成功处理完的fd */
struct  remainfd {
	bool	init;		/* 此结构体是否正确被初始化 */
	int	capacity;	/* 能够保存未处理完毕的fd的总数 */
	int	wcnt;		/* 写事件没有处理完的fd的总计数 */
	int	rcnt;		/* 读事件没有处理完的fd的总计数 */
	int	packcnt;	/* 打包没有处理完的fd的总计数 */
	int	parscnt;	/* 解析没有处理完的fd的总计数 */
	int*	wfda;		/* 用于保存写事件没有处理完的fd的首地址 */
	int*	rfda;		/* 用于保存读事件没有处理完的fd的首地址 */
	int*	packfda;	/* 用于保存打包没有处理完的fd的首地址 */
	int*	parsfda;	/* 用于保存解析没有处理完的fd的首地址 */
};

/* 监听描述符的相关信息 */
struct listenfd {
	int		counter;			/* 监听fd的计数器 */
	struct cbinfo	finishedcb[LISTEN_SIZE];	/* 每个fd对应的回调函数 */
	struct comm_tcp	commtcp[LISTEN_SIZE];		/* 套接字的相关信息 */
};

/* 每个fd对应的数据相关信息 */
struct comm_data {
	struct comm_queue	send_queue;	/* 存放用户传递但并未打包的数据 */
	struct comm_list	send_list;	/* 当send_queue队列已满时放入此链表中 */
	struct comm_cache	recv_cache;	/* 存放接收但并未解析的数据 read函数使用 */
	struct comm_cache	send_cache;	/* 存放需要发送并已经打包的数据 write函数使用 package函数使用 */
	struct comm_lock	sendlock;	/* send_queue的锁 */
	struct cbinfo		finishedcb;	/* 此描述符监听事件发生时相应的回调函数信息 */
	struct comm_tcp		commtcp;	/* 套接字相关信息 */
	struct mfptp_parser	parser;		/* 解析器 */
	struct mfptp_packager	packager;	/* 打包器 */
};

/* 事件相关信息 */
struct comm_event {
	bool		init;			/* 本结构体是否正确初始化 */
	int		fds;			/* 客户端连接到服务器的fd的总个数即数组data里面数据有效个数 */
	int		fda[EPOLL_SIZE/4];	/* 从管道读取的所有fd，即此次需要发送数据的所有fd */
	intptr_t	data[EPOLL_SIZE];	/* 保存接收发送数据的结构体 */
	struct listenfd	listenfd;		/* 所有监听描述符的信息 */
	struct remainfd remainfd;		/* 未处理完的fd */
	struct cbinfo	timeoutcb;		/* epoll_wait超时的回调函数 */
	struct comm_context* commctx;
};


/*************************************** 数据结构体相关操作 内部使用 ********************************************/

/* 分配一个comm_message的结构体 @size：结构体中消息内容的大小 */
bool new_commmsg(struct comm_message **message, int size);

/* 拷贝一份comm_message结构体的数据 @destmsg:拷贝的目的结构体 @srcmsg:拷贝的源结构体 */
void copy_commmsg(struct comm_message* destmsg, const struct comm_message* srcmsg);

/* 销毁comm_message的结构体 @arg:类型为struct comm_message，这里为void是为了符合回调函数的参数模式 */
void free_commmsg(void* arg);

/* 初始化一个struct remainfd结构体 */
bool init_remainfd(struct remainfd *remainfd);

/* 将扩容的部分恢复到原始的大小 */
bool restore_remainfd(struct remainfd *remainfd);

/* 释放struct remainfd里面分配的内存 */
void free_remainfd(struct remainfd *remainfd);

/* 添加一个未处理完的fd @fd:待添加的fd @type:待添加的fd的类型[remainfd_type] */
bool add_remainfd(struct remainfd *remainfd, int fd, int type);

/* 删除一个未处理完的fd @fd:待删除的fd @type:待添加的fd的类型[remainfd_type] */
void del_remainfd(struct remainfd *remainfd, int fd, int type);

#ifdef __cplusplus
	}
#endif

#endif /* ifndef __COMM_STRUCTURE_H__ */
