/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_structure.h"

/* 初始化一个fd的数据结构体，并将此fd添加到epoll的监控中 */
struct comm_data*  commdata_init(struct comm_context* commctx, struct portinfo* portinfo,  struct cbinfo*  finishedcb)
{
	assert(commctx && portinfo);
	int			nodesize = sizeof(intptr_t);
	int			flag = 0;
	bool			retval = false;
	struct comm_data	*commdata = NULL;

	New(commdata);
	if (unlikely(!commctx)) {
		goto error;
	}
	retval = commqueue_init(&commdata->recv_queue, QUEUE_CAPACITY, nodesize);
	if (unlikely(!retval)) {
		goto error;
	}
	retval = commqueue_init(&commdata->send_queue, QUEUE_CAPACITY, nodesize);
	if (unlikely(!retval)) {
		goto error;
	}
	retval = commcache_init(&commdata->recv_buff, CACHE_SIZE);
	if (unlikely(!retval)) {
		goto error;
	}

	retval = commcache_init(&commdata->send_buff, CACHE_SIZE);
	if (unlikely(!retval)) {
		goto error;
	}
	retval = commlock_init(&commdata->sendlock);
	if( unlikely(!retval)) {
		goto error;
	}

	if (portinfo->type == COMM_BIND) {
		flag = EPOLLIN | EPOLLET;
	} else {
		flag = EPOLLIN | EPOLLOUT | EPOLLET;
	}
	/* 监听套接字的时间太晚，可能会出现事件已发生，但是epoll却没监听到 */
	retval = commepoll_add(&commctx->commepoll, portinfo->fd, flag);
	if( unlikely(!retval) ){
		goto error;
	}

	commdata->commctx = commctx;
	memcpy(&commdata->portinfo, portinfo, sizeof(*portinfo));
	if (finishedcb) {
		memcpy(&commdata->finishedcb, finishedcb, sizeof(*finishedcb));
	}
	return commdata;

error:
	commlock_destroy(&commdata->sendlock);
	commqueue_destroy(&commdata->recv_queue);
	commqueue_destroy(&commdata->send_queue);
	commepoll_destroy(&commctx->commepoll);
	
	commcache_free(&commdata->recv_buff);
	commcache_free(&commdata->send_buff);
	Free(commdata);
	return NULL;
}

/* 销毁一个struct comm_data的结构体 */
void commdata_destroy(struct comm_data *commdata)
{
	if (likely(commdata)) {
		commlock_destroy(&commdata->sendlock);
		commqueue_destroy(&commdata->recv_queue);
		commqueue_destroy(&commdata->send_queue);
		
		commcache_free(&commdata->recv_buff);
		commcache_free(&commdata->send_buff);

		commepoll_del(&commdata->commctx->commepoll, commdata->portinfo.fd, -1);
		Free(commdata);
	}

	return ;
}

/* 分配一个comm_message的结构体 @size：结构体中消息内容的大小 */
inline struct comm_message* new_commmsg(int size)
{
	struct comm_message *message = NULL;
	New(message);
	if (!message) {
		return NULL;
	}
	NewArray(message->content, size);
	if (!message->content) {
		Free(message);
		return NULL;
	}
	return message;
}

/* 拷贝一份comm_message结构体的数据 */
inline void copy_commmsg(struct comm_message* destmsg, const struct comm_message* srcmsg)
{
	assert(destmsg && srcmsg && destmsg->content && srcmsg->content);
	destmsg->fd = srcmsg->fd;
	destmsg->size = srcmsg->size;
	destmsg->encrypt = srcmsg->encrypt;
	destmsg->compress = srcmsg->compress;
	memcpy(destmsg->content, srcmsg->content, srcmsg->size);
}

/* 销毁comm_message的结构体 */
inline void free_commmsg(struct comm_message* message)
{
	if (message) {
		Free(message->content);
		Free(message);
	}
}
