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
	struct comm_pipe	commpipe;		/* 关于管道的相关信息 */
	struct comm_lock	recvlock;		/* 用来同步接收队列的锁*/
	struct comm_lock	statlock;		/* 用来同步stat的状态 */
	struct comm_epoll	commepoll;		/* epoll监听事件的相关信息 */
	struct comm_list	msghead;		/* 消息结构体链表的头节点 */
	enum {
		COMM_STAT_NONE,
		COMM_STAT_INIT,
		COMM_STAT_RUN,
		COMM_STAT_STOP
	}		stat;				/* 线程的状态 */
};


/********************************  以下为内部结构体 外部无需关心 ********************************************/

/* 此结构体保存发送接收时没成功处理完的fd */
struct  remainfd {
	int	wpcnt;				/* 没有及时发送消息或没有打包完毕[发送队列里面还存在没有打包完的数据]fd的总计数 */
	int	rpcnt;				/* 没有及时接收消息或没有解析完毕[接收缓冲里面还存在没有解析完的数据]fd的总计数 */
	int	wpfda[EPOLL_SIZE/4];		/* 没有及时发送消息或者没有打包完的fd都会先打包然后发送数据， 所以无需区分类型 */
	struct rpfds{				/* 没有及时接收到消息的fd会去接收消息然后解析，没有解析完毕的fd只需进行解析就行了*/
		enum {
			REMAIN_READ = 0x01,	/* 残留的对应fd属于读事件失败，尝试重读 */
			REMAIN_PARSE		/* 残留的对应fd属于没有解析完毕，继续解析 */
		}	type[EPOLL_SIZE/4];
		int	fda[EPOLL_SIZE/4];	/* 保存出现意外没有及时读取消息或没有解析完数据的fd */
	}rpfds;					/* 读和解析残留的所有fd */
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




#ifdef __cplusplus
	}
#endif

#endif /* ifndef __COMM_STRUCTURE_H__ */
