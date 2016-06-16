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

#include "netmod.h"

int get_thread_id_by(int threads, const char *id);

int event_dispenser_startup(event_ctx_t *ev_ctx, int threads);
#endif

