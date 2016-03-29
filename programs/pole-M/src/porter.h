#pragma once

#include <pthread.h>

#include "pipe.h"
#include "xmq.h"
#include "netmod.h"

typedef struct porter_info_struct
{
	char            peer[256];
	xlist_t         list;

	pthread_t       tid;

	xmq_consumer_t  *consumer;
	event_ctx_t     *transceiver;

	event_t         *sending;	// 保存每次正常发送的数据包;
	event_t         *reserved;	// 保存EV_FAIL和EV_DUMP时的上次数据包;

	pipe_t          todump;		// 专门处理EV_DUMP的事件;
	pipe_t          events;		// 处理一般性的事件请求;

	uint64_t        start_seq;
} porter_info_t;

extern xlist_t          porters_list_head;
extern pthread_mutex_t  porters_list_lock;

extern void porters_list_init(void);

extern void porters_list_destroy(void);

extern porter_info_t *look_up_porter(const char *peername);

extern porter_info_t *porter_create(const char *peer, xmq_consumer_t *consumer, event_ctx_t *transceiver, uint64_t start_seq);

extern void porter_destroy(porter_info_t *porter);

extern int porter_work_start(porter_info_t *info);

