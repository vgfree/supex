/***********************************************************************
 * Copyright (C), 2010-2016,
 * ShangHai Language Mirror Automobile Information Technology Co.,Ltd.
 *
 *   File Name: event_handler.h
 *      Author: liubaoan@mirrtalk.com (刘绍宸)
 *     Version: V1.0
 *        Date: 2016/02/25
 *    Business:
 *        1. 将从事件分发器(event_dispender)获取的指定网络事件,进行相关
 *           业务处理,并将处理结果发送到网络;
 *
 *        2. 当程序启动时,每个线程内部,都会启动一个协程集群,用于处理这些事件,
 *           并记录相关信息,每个客户端都会在服务器保留一个 client_info_t.
 *           直到客户端出现严重错误,并将该错误告知服务器,此时,从线程中
 *           删除该指定客户端的状态信息;
 *
 *        3. 服务器端如果意外宕掉,或被KILL,重新启动时,它会自动加载上次
 *           已同步的所有客户端状态信息,并可以正常同步数据;
 *
 *      Others:
 *        (null)
 *
 *     History:
 *        Date:           Author:            Modification:
 *
 **********************************************************************/

#ifndef _EVENT_HANDLER_H_
#define _EVENT_HANDLER_H_

#include "base/hashmap.h"
#include "xlist.h"
#include "netmod.h"
#include "xmq.h"
#include "event_pipe.h"
#include "evcoro_scheduler.h"

#ifdef DEBUG
  #define DBG_PRINT(format)             printf(format)
  #define DBG_PRINTS(format, ...)       printf(format, __VA_ARGS__)
#else
  #define DBG_PRINT(format)
  #define DBG_PRINTS(format, ...)
#endif

typedef struct event_node_t event_node_t;
typedef struct client_info_t client_info_t;
typedef struct thread_member_t thread_member_t;

/* 这个结构体在主线程中创建,启动线程后,将指针传入各自子线程 */
struct thread_member_t
{
	xlist_t                 list;		// 链接器,多个线程之间是否需要连接,待定;

	int                     tid;		// 当前线程的标识(0 ~ threads-1);
	int                     threads;	// 具体的线程数;
	event_ctx_t             *ev_ctx;	// 事件上下文指针,全局共享的,用于接收和发送客户端信息;
	xmq_ctx_t               *xmq_ctx;	// XMQ 队列上下文对象,用于线程内部创建消费者对象;

	xlist_t                 list_events;	// 事件链表,来自客户端的事件请求,经过求余和HASH算法之后,被添加进来;
	xlist_t                 list_clinfo;	// 客户端状态信息链表,被服务端实时更新;
	int                     size_clinfo;	// 当前线程对象内部有几个客户端对象;
	hashmap_t               *hash_clinfo;	// 客户端状态信息的HASH表,一个线程保存了多个客户端状态;

	struct
	evcoro_scheduler        *evcoro_sched;	// 协程调度器,每个线程都有一个;
};

/* 保存客户端操作对象及状态信息*/
struct client_info_t
{
	xlist_t         list;		// 链接器,用于客户端状态之间的对象连接;

	/* 状态信息 */
	char            id[IDENTITY_SIZE];	// 客户端标识
	int             tid;			// 当前客户节点隶属于哪个线程(0 ~ threads-1);

	event_t         *ev_send_okay;		// 上次成功发送的事件(INCR_REP|DUMP_REQ)的缓存,动态更新;
	event_t         *ev_fail;		// 增量同步时,客户端发过来含有上次执行出错的事件;

	/* 处理DUMP请求时的参数 */
	event_t         *ev_incr_wait;	/* 执行DUMP请求时,缓存的上次执行增量的返回事件,
					 * 只有当所有的DUMP请求,处理完成后,才可以继续执行增量同步; */
	evpipe_t        *evp_dumps;	// 临时存储待DUMP的事件对象;

	/* 操作队列 */
	xmq_consumer_t  *consumer;	// 每个客户节点对应一个消费者,用于同步队列的数据;
	uint64_t        sync_seq;	// 当前同步的序列;
};

/* 网络事件包,每个线程都会有这样一个链表,线程不断迭代这个链表将所有的事件,添加到协程中处理;
 *   因在线程内部操作,所以 不需要加锁; */
struct event_node_t
{
	xlist_t         list;		// 链接器

	thread_member_t *thrd;		// 用于协程中处理事件;
	event_t         *ev;		// 临时保留来自客户端的事件对象指针;
};

/* 线程对象相关操作 */
/* ============================================================= */

/* 创建一个线程对象,并将KEY:id(客户端标识"xxx"通过对线程个数<threads>求余获得的数字)
 * 和VALUE:具体的线程对象地址,添加到全局的线程HASH对象中. */
thread_member_t *thread_member_create(hashmap_t *hash_thrd, int id, int threads,
	event_ctx_t *evctx, xmq_ctx_t *xctx);

int thread_member_destroy(thread_member_t *thrd);

/* 通过客户端ID获取它所对应的线程对象. */
thread_member_t *thread_member_getby(hashmap_t *hash_thrd, int id);

/* 往线程里添加新的事件. */
int thread_member_push_event(thread_member_t *thrd, event_t *ev);

/* 客户端信息相关操作 */
/* ============================================================= */
client_info_t *client_info_create(const char *id);

/* 从某个线程的客户端状态HASH表中查找标识为(id)的客户端对象并返回. */
client_info_t *client_info_get_by(hashmap_t *hash_clinfo, const char *id);

/* 在指定线程对象中,添加一个标识为(id)的客户端状态对象. */
int client_info_add(thread_member_t *thrd, const char *id, client_info_t *clinfo);

/* 通过客户端标识,从客户端HASH表中删除当前客户端状态信息,并从链表中删除. */
int client_info_delete_by(thread_member_t *thrd, const char *id);

/* 获取当前线程对象中客户端的个数 */
int client_info_length(thread_member_t *thrd);

int client_info_destroy(client_info_t *clinfo);

/* 事件处理启动函数; */
/* ============================================================= */
/* 启动事件处理机 */
int event_handler_startup(event_ctx_t *evctx, xmq_ctx_t *xctx, int threads);

/* PUSH某个新的网络事件到线程号为 thread 的线程内部. */
int push_event_to_thread(event_t *ev, int thread);

int event_handler_destroy();
#endif	/* ifndef _EVENT_HANDLER_H_ */

