/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#pragma once

#include "mfptp_protocol/mfptp_parse.h"
#include "mfptp_protocol/mfptp_package.h"

#include "comm_utils.h"
#include "comm_epoll.h"
#include "comm_tcp.h"
#include "comm_message.h"
#include "comm_lock.h"
#include "comm_queue.h"
#include "comm_list.h"
#include "comm_cache.h"
#include "comm_pipe.h"

#ifdef __cplusplus
extern "C" {
#endif

struct comm_data
{
	struct comm_lock        recvlock;	/* 用来同步接收队列的锁*/
	struct comm_queue       recvqueue;	/* 存放接收并解析好的数据 */
	struct comm_list        recvlist;	/* 当recvqueue已满时放入此链表中 */

	struct comm_lock        sendlock;	/* send_list的锁 */
	struct comm_queue       sendqueue;	/* 存放用户传递进来未打包的原始数据 */
	struct comm_list        sendlist;	/* 当send_queue队列已满时放入此链表中 */

	struct comm_cache       recv_cache;	/* 存放read函数接收但并未解析的数据 */
	struct comm_cache       send_cache;	/* 存放已打包好待发送的数据 */

	struct mfptp_parser     parser;		/* 解析器 */
	struct mfptp_packager   packager;	/* 打包器 */
};

/* 初始化一个fd的数据结构体struct comm_data */
bool commdata_init(struct comm_data *commdata);

/* 销毁一个fd的数据结构体struct comm_data*/
void commdata_away(struct comm_data *commdata);

/* 开始打包一个fd的数据,@return -1出错，>=0成功个数 */
int commdata_package(struct comm_data *commdata);

/* 开始解析一个fd的数据,@return -1出错，>=0成功个数 */
int commdata_parse(struct comm_data *commdata);

enum STEP_CODE
{
	STEP_WAIT = 0,
	STEP_HAND,
	STEP_STOP,
};

// TODO
#ifndef COMM_FCB
struct comm_context;
/* 回调函数的相关信息 */
typedef void (*COMM_FCB)(struct comm_context *commctx, struct comm_tcp *commtcp, void *usr);
struct cbinfo
{
	int             timeout;
	COMM_FCB        callback;		/* 相关的回调函数 */
	void            *usr;			/* 用户的参数 */
};
#endif
/* 绑定监听描述符的相关信息[fd类型为COMM_BIND] */
struct bindfd_info
{
	struct cbinfo   finishedcb;	/* 每个fd对应的回调函数 */
	struct comm_tcp commtcp;	/* 套接字的相关信息 */
	enum STEP_CODE  workstep;
};

/* 主动链接或被动连接的fd相关信息[fd的类型为COMM_CONNECT, COMM_ACCEPT] */
struct connfd_info
{
	struct cbinfo           finishedcb;	/* 此描述符监听事件发生时相应的回调函数信息 */
	struct comm_tcp         commtcp;	/* 套接字相关信息 */
	enum STEP_CODE          workstep;

	struct comm_data        commdata;
};

/* @返回值：true:连接正常,false:连接异常 */
bool commconnfd_send(struct connfd_info *connfd);

/* @返回值：true:连接正常,false:连接异常 */
bool commconnfd_recv(struct connfd_info *connfd);

enum
{
	EVT_TYPE_NULL = 0x00,
	EVT_TYPE_PIPE = 0x01,
	EVT_TYPE_SOCK = 0x10,
};

/* 事件相关信息 */
struct comm_evts
{
	bool                    init;			/* 本结构体是否正确初始化 */
	bool                    canfree;

	int                     connfdcnt;		/* 主动[connect]或被动[accept]连接的fd的总数  */
	int                     bindfdcnt;		/* 绑定监听[bind&&listen]fd的总数 */
	struct connfd_info      *connfd[EPOLL_SIZE];	/* 主动或被动连接fd的所有信息 */
	struct bindfd_info      bindfd[LISTEN_SIZE];	/* 绑定监听fd的所有信息 */
	// TODO:需要lock

	struct comm_pipe        sendpipe;		/* 记录有发送数据的fd信息 */
	// int			sendevfd;
	struct comm_pipe        recvpipe;		/* 记录有接收数据的fd信息 */
	// int			recvevfd;

	struct comm_pipe        cmdspipe;		/* 记录有open/close的fd信息 */

	struct comm_epoll       commepoll;		/* epoll监听事件的相关信息 */

	void                    *commctx;
};

/* 初始化事件结构体 */
struct comm_evts        *commevts_make(struct comm_evts *commevts);

/* 销毁一个事件结构体 */
void commevts_free(struct comm_evts *commevts);

/* 新增一个socket */
bool commevts_socket(struct comm_evts *commevts, struct comm_tcp *commtcp, struct cbinfo *finishedcb);

bool commevts_push(struct comm_evts *commevts, struct comm_message *message);

bool commevts_pull(struct comm_evts *commevts, struct comm_message *message);

void commevts_close(struct comm_evts *commevts, int fd);

/* 执行一次事件循环 */
void commevts_once(struct comm_evts *commevts);

#ifdef __cplusplus
}
#endif

