/***********************************************************************
 * Copyright (C), 2010-2016,
 * ShangHai Language Mirror Automobile Information Technology Co.,Ltd.
 *
 *   File Name: event_dispender.h
 *      Author: liubaoan@mirrtalk.com (刘绍宸)
 *     Version: V1.0
 *        Date: 2016/02/25
 *    Business:
 *        1. 服务器在接收到来自客户端的事件请求后,通过 hit_thread_by_clientid()
 *           将指定的事件仍到特定的线程,并由线程内部的协程进行功能性操作;
 *        2.
 *        3.
 *
 *      Others:
 *        (null)
 *
 *     History:
 *        Date:           Author:            Modification:
 **********************************************************************/

#ifndef _EVENT_DISPENSER_H_
#define _EVENT_DISPENSER_H_

#include "thread_pool_loop/tlpool.h"
#include "base/hashmap.h"
#include "netmod.h"
#include "xmq.h"

void event_dispenser_startup(xmq_ctx_t *xmq_ctx, evt_ctx_t *evt_ctx, tlpool_t *tlpool, hashmap_t *hmap, int threads);
#endif

