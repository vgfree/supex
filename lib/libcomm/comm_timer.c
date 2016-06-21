/*********************************************************************************************/
/************************	Created by 许莉 on 16/06/15.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_timer.h"
#include <sys/time.h>

#define COMMTIMER_OFFSET	({ struct comm_timer commtimer;				 \
				   get_member_offset(&commtimer, &commtimer.ntimer);	\
				})

struct comm_timer* commtimer_create(uint64_t value, uint64_t interval, EventCB callback, void *usr)
{
	assert(callback && (value > 0));
	struct comm_timer *commtimer = NULL;
	New(commtimer);
	if (commtimer) {
		commtimer->value.tv_sec = value/1000;
		commtimer->value.tv_usec = (value%1000) * 1000;
		commtimer->callback = callback;
		commtimer->usr = usr;
		if (interval > 0) {
			commtimer->interval.tv_sec = interval/1000;
			commtimer->interval.tv_usec = (interval%1000) * 1000;
		}
		commtimer->init = true;
	} 
	return commtimer;
}

void  commtimer_destroy(struct comm_timer *commtimer)
{
	if (commtimer && commtimer->init) {
		Free(commtimer);
	}
}


void commtimer_start(struct comm_timer *commtimer, struct comm_list *timerhead) 
{
	assert(timerhead && commtimer && commtimer->init);
	/* 开始计时 */
	gettimeofday(&commtimer->start, NULL);
	commlist_push(timerhead, &commtimer->ntimer);
}

void commtimer_stop(struct comm_timer *commtimer, struct comm_list *timerhead) 
{
	assert(timerhead && commtimer && commtimer->init);
	/* 恢复计时 */
	memset(&commtimer->start, 0, sizeof(commtimer->start));
	commlist_delete(timerhead, &commtimer->ntimer);
}

void commtimer_schedule(struct comm_list *timerhead)
{
	assert(timerhead);

	struct timeval		end = {};
	struct timeval		diff = {};
	struct comm_timer*	commtimer = NULL;
	struct comm_list*	list = NULL;

	while (commlist_get(timerhead, &list)) {
		commtimer = (struct comm_timer*)get_container_addr(list, COMMTIMER_OFFSET);
		gettimeofday(&end, NULL);
		diff.tv_sec = end.tv_sec - commtimer->start.tv_sec;
		diff.tv_usec = end.tv_usec - commtimer->start.tv_usec;
		if (diff.tv_sec > commtimer->value.tv_sec || (diff.tv_sec == commtimer->value.tv_sec && diff.tv_usec > commtimer->value.tv_usec)) {
			commtimer->callback(commtimer->usr);
			if (commtimer->interval.tv_sec > 0 || commtimer->interval.tv_usec > 0) {
				/* 间隔触发事件 */
				gettimeofday(&commtimer->start, NULL);
				commtimer->value.tv_sec = commtimer->interval.tv_sec;
				commtimer->value.tv_usec = commtimer->interval.tv_usec;
			} else {
				/* 只触发一次事件 则触发时间之后将timer移除链表 */
				commlist_delete(timerhead, &commtimer->ntimer);
			}
		}
	}
}
