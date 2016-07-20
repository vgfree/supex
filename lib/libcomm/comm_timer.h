/*********************************************************************************************/
/************************	Created by 许莉 on 16/06/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_TIMER_H__
#define __COMM_TIMER_H__

#include "comm_list.h"
#include "comm_utils.h"
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

struct comm_timer;

/* 时间事件回调函数模型 */
typedef void (*EventCB)(struct comm_timer *commtimer, struct comm_list *timerhead, void *arg);

struct comm_timer {
	bool	init;			/* 此结构体是否被正确的初始化 */
	void*	usr;			/* 事件回调函数的参数 */
	EventCB callback;		/* 触发事件的回调函数 */
	struct timeval start;		/* 开始计时器时的事件记录*/
	struct timeval interval;	/* 每间隔多长时间再次触发事件,为0则只触发一次 */
	struct timeval value;		/* 从开始timer起到下一次触发事件的时间长度 */
	struct list_node ntimer;	/* 链表节点结构体,指向下一个timer[用户无需设置此值] */
};


/***********************************************************************************
* 功能:创建一个timer结构体
* @vaule:距离下一次触发事件多长时间[单位:ms 1s = 1000ms]
* @interval: 每间隔多长事件触发一次事件,0则代表此事件只会被触发一次
* @callback:触发的事件回调函数
* @usr:事件回调函数的参数
* @返回值:成功则返回一个timer结构体 失败返回NULL
***********************************************************************************/
struct comm_timer* commtimer_create(uint64_t vaule, uint64_t interval, EventCB callback, void *usr);

/***********************************************************************************
* 功能: 销毁一个commtimer结构体并将其从timer链表中删除
***********************************************************************************/
void commtimer_destroy(struct comm_timer *commtimer);

/***********************************************************************************
* 功能: 启动一个timer[将一个timer加入到timer链表中,等待被调度]
* @timerhead: timer链表的头节点
***********************************************************************************/
void commtimer_start(struct comm_timer *commtimer, struct comm_list *timerhead);

/***********************************************************************************
* 功能: 停止一个timer[将一个timer从timer链表中删除]
* @timerhead: timer链表的头节点
***********************************************************************************/
void commtimer_stop(struct comm_timer *commtimer, struct comm_list *timerhead);

/***********************************************************************************
* 功能: timer事件的调度器
* @timerhead: timer链表的头节点
***********************************************************************************/
void commtimer_scheduler(struct comm_list *timerhead);



#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_TIMER_H__ */
