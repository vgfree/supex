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
#include "base/xlist.h"
#include "netmod.h"
#include "xmq.h"

#define OPEN_BATCH
#define MAX_SYNC_STEP 10

typedef struct client_info client_info_t;

struct online_task
{
	client_info_t *addr;
};

/* 保存客户端操作对象及状态信息*/
struct client_info
{
	evt_ctx_t               *evt_ctx;	// 事件上下文指针,全局共享的,用于接收和发送客户端信息;
	xmq_ctx_t               *xmq_ctx;	// XMQ 队列上下文对象,用于线程内部创建消费者对象;
	hashmap_t               *hmap;

	/* 状态信息 */
	char                    id[IDENTITY_SIZE];	// 客户端标识

	enum
	{
		EVT_STATE_INCR = 0,
		EVT_STATE_WAIT,
		EVT_STATE_DUMP,
		EVT_STATE_EXIT,
		EVT_STATE_RUIN,
	}                       state;
	size_t                  waits;

	/* 只有当所有的DUMP请求,处理完成后,才可以继续执行增量同步; */
	struct queue_list       qdump;	// 临时存储待DUMP的事件对象;
	struct queue_list       qevts;	// 临时存储待REP的事件对象;
	struct queue_list       qincr;	// 临时存储待REQ的事件对象;

	/* 操作队列 */
	xmq_consumer_t          *consumer;	// 每个客户节点对应一个消费者,用于同步队列的数据;
};

/* 客户端信息相关操作 */
/* ============================================================= */
client_info_t *client_info_create(const char *id);

void client_info_init(client_info_t *p_info);

int client_info_destroy(client_info_t *clinfo);

/* 事件处理启动函数; */
/* ============================================================= */
/* 启动事件处理机 */
void event_handler_startup(void *data);
#endif	/* ifndef _EVENT_HANDLER_H_ */

