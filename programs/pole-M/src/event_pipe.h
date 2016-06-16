#ifndef _EVENT_PIPE_H_
#define _EVENT_PIPE_H_

#include "xlist.h"
#include "netmod.h"
#include <pthread.h>

typedef struct evpipe_t evpipe_t;
typedef struct evpipe_ele_t evpipe_ele_t;

// 因为是在线程内部操作每个节点,所以不需要加锁;
struct evpipe_t
{
	xlist_t head;
	size_t  ele_counts;
	//	int              okay;
	//	pthread_cond_t  *cond;
};

// 事件管道类型的每个成员节点;
struct evpipe_ele_t
{
	xlist_t node;
	event_t *ev;
};

evpipe_t *evpipe_init();

void evpipe_release(evpipe_t *pipe);

int evpipe_push_head(evpipe_t *pipe, event_t *ev);

int evpipe_push_tail(evpipe_t *pipe, event_t *ev);

int evpipe_length(evpipe_t *pipe);

event_t *evpipe_pull(evpipe_t *pipe);
#endif /* ifndef _EVENT_PIPE_H_ */

