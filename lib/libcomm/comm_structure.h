/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_STRUCTURE_H__
#define __COMM_STRUCTURE_H__

#include "comm_timer.h"
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

#define EPOLL_SIZE              (1082*100)	/* 允许EPOLL能够监听的描述符的最大个数 */
#define LISTEN_SIZE             10	/* 允许监听fd的最大个数 */
#define COMM_FRAMES             13	/* 允许一个包最大的总帧数 */
#define REMAINFD_INCREASE_SIZE  10	/* remainfd扩容时的增加幅度 */

/* 获取struct comm_message结构体中成员变量list的偏移大小 */
#define COMMMSG_OFFSET			\
	({ struct comm_message message;	\
	   get_member_offset(&message, &message.list); })

struct comm_event;
struct comm_context;
/********************************  以下为外部结构体 外部使用需要了解 ****************************************/

/* 回调函数的原型 */
typedef void (*CommCB)(struct comm_context *commctx, struct comm_tcp *commtcp, void *usr);

/* 回调函数的相关信息 */
struct cbinfo
{
	int     timeout;
	CommCB  callback;			/* 相关的回调函数 */
	void    *usr;				/* 用户的参数 */
};

/* 数据包的设置 */
struct comm_package
{
	int     dsize;					/* 数据总大小 */
	int     frames;					/* 总帧数 */
	int     packages;				/* 总包数 */
	int     frame_size[COMM_FRAMES];		/* 每个帧数据大小 */
	int     frame_offset[COMM_FRAMES];		/* 每个帧的偏移 */
	int     frames_of_package[COMM_FRAMES];		/* 每个包中帧的多少 */
};

/* 发送接收数据的结构体 */
struct comm_message
{
	int                     fd;			/* 消息对应的描述符 */
	int                     config;			/* 消息的加密压缩格式 */
	int                     socket_type;		/* 消息套接字的类型[要使用默认值的时候必须将此值设置为-1] */
	char                    *content;		/* 消息的内容首地址 */
	struct comm_package     package;		/* 消息包的设置 */
	struct comm_list        list;			/* 链表节点 */
};

/* 通信模块的上下文环境结构体 */
struct comm_context
{
	pthread_t               ptid;			/* 新线程的pid */
	struct comm_event       *commevent;		/* 事件驱动的相关信息 */
	struct comm_list	timerhead;		/* 计时器事件链表 */
	struct comm_timer*	pipetimer;		/* 定时读取管道信息的计时器 */
	struct comm_queue       recvqueue;		/* 存放接收并解析好的数据 */
	struct comm_cache	cache;			/* 所有数据进行加密压缩解密解压的过渡缓冲区 */
	struct comm_list        recvlist;		/* 当recvqueue已满时放入此链表中 */
	struct comm_pipe        commpipe;		/* 关于管道的相关信息 */
	struct comm_lock        recvlock;		/* 用来同步接收队列的锁*/
	struct comm_lock        statlock;		/* 用来同步stat的状态 */
	struct comm_epoll       commepoll;		/* epoll监听事件的相关信息 */
	enum
	{
		COMM_STAT_NONE,
		COMM_STAT_INIT,
		COMM_STAT_RUN,
		COMM_STAT_STOP
	}                       stat;			/* 线程的状态 */
};

/********************************  以下为内部结构体 外部无需关心 ********************************************/

/* 残留的fd的类型 */
enum remainfd_type
{
	REMAINFD_LISTEN = 0x01,
	REMAINFD_WRITE,
	REMAINFD_READ,
	REMAINFD_PARSE,
	REMAINFD_PACKAGE
};

/* 数组的每个元素分别代表的是:读，写，解析，打包，监听的相关fd的信息 */
struct remainfd
{
	bool    init;		/* 此结构体是否被正确的初始化 */
	int     capacity[5];	/* 每个元素分别代表能够保存LISTEN,REDA,PARSE,PACKAGE,WRITE类型的fd的容量 */
	int     cnt[5];		/* 每个元素分别代表已保存LISTEN,READ,PARSE,PACKAGE,WRITE类型的fd的总数 */
	int     circle[5];	/* 每个元素分别代表此次已处理LISTEN,READ,PARSE,PACKAGE,WRITE类型的fd的下标,下一次继续执行下一个相关fd */
	int     *fda[5];	/* 每个元素分别代表用于保存LISTEN,READ,PARSE,PACKAGE,WRITE类型的fd的首地址 */
};

/* 绑定监听描述符的相关信息[fd类型为COMM_BIND] */
struct bindfd_info
{
	struct cbinfo   finishedcb;	/* 每个fd对应的回调函数 */
	struct comm_tcp commtcp;	/* 套接字的相关信息 */
};

/* 主动链接或被动连接的fd相关信息[fd的类型为COMM_CONNECT, COMM_ACCEPT] */
struct connfd_info
{
	struct comm_queue       send_queue;	/* 存放用户传递进来未打包的原始数据 */
	struct comm_list        send_list;	/* 当send_queue队列已满时放入此链表中 */
	struct comm_cache       recv_cache;	/* 存放read函数接收但并未解析的数据 */
	struct comm_cache       send_cache;	/* 存放已打包好待发送的数据 */
	struct comm_lock        sendlock;	/* send_queue的锁 */
	struct comm_timer*	commtimer;	/* 计时器,当fd标志位设置为CONNECNT_ANYWAY，若服务器突然断开,此计时器就会定时的去连接服务器 */
	struct comm_event*	commevent;	
	struct cbinfo           finishedcb;	/* 此描述符监听事件发生时相应的回调函数信息 */
	struct comm_tcp         commtcp;	/* 套接字相关信息 */
	struct mfptp_parser     parser;		/* 解析器 */
	struct mfptp_packager   packager;	/* 打包器 */
};

/* 事件相关信息 */
struct comm_event
{
	bool                    init;			/* 本结构体是否正确初始化 */
	int                     connfdcnt;		/* 主动[connect]或被动[accept]连接的fd的总数  */
	int                     bindfdcnt;		/* 绑定监听[bind&&listen]fd的总数 */
	struct connfd_info      *connfd[EPOLL_SIZE];	/* 主动或被动连接fd的所有信息 */
	struct bindfd_info      bindfd[LISTEN_SIZE];	/* 绑定监听fd的所有信息 */
	struct remainfd         remainfd;		/* 所有待处理事件的fd */
	struct cbinfo           timeoutcb;		/* epoll_wait超时的回调函数 */
	struct comm_context     *commctx;
};

/*************************************** 数据结构体相关操作 内部使用 ********************************************/

/* 分配一个comm_message的结构体 @size：结构体中消息内容的大小 */
bool new_commmsg(struct comm_message **message, int size);

/* 拷贝一份comm_message结构体的数据 @destmsg:拷贝的目的结构体 @srcmsg:拷贝的源结构体 */
void copy_commmsg(struct comm_message *destmsg, const struct comm_message *srcmsg);

/* 销毁comm_message的结构体 @arg:类型为struct comm_message，这里为void是为了符合回调函数的参数模式 */
void free_commmsg(void *arg);

/* 初始化一个struct remainfd结构体 */
bool init_remainfd(struct remainfd *remainfd);

/* 将扩容的部分恢复到原始的大小 */
void restore_remainfd(struct remainfd *remainfd);

/* 释放struct remainfd里面分配的内存 */
void free_remainfd(struct remainfd *remainfd);

/* 添加一个未处理完的fd @fd:待添加的fd @type:待添加的fd的类型[remainfd_type] */
bool add_remainfd(struct remainfd *remainfd, int fd, int type);

/* 删除一个未处理完的fd @fd:待删除的fd @type:待添加的fd的类型[remainfd_type] */
void del_remainfd(struct remainfd *remainfd, int fd, int type);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __COMM_STRUCTURE_H__ */

