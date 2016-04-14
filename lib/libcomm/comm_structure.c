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

	if (likely(commctx->watchcnt < EPOLL_SIZE)) {
		int flag = 0;
		if (portinfo->type == COMM_BIND) {
			flag = EPOLLIN | EPOLLET;
		} else {
			flag = EPOLLIN | EPOLLOUT | EPOLLET;
		}
		retval = add_epoll(commctx->epfd, portinfo->fd, flag);
		if( unlikely(!retval) ){
			goto error;
		}
		commctx->watchcnt ++;
	}

	commdata->commctx = commctx;
	memcpy(&commdata->portinfo, portinfo, sizeof(*portinfo));
	if (finishedcb) {
		memcpy(&commdata->finishedcb, finishedcb, sizeof(*finishedcb));
	}
	return commdata;

error:
	commqueue_destroy(&commdata->recv_queue);
	commqueue_destroy(&commdata->send_queue);
	
	commcache_free(&commdata->recv_buff);
	commcache_free(&commdata->send_buff);
	Free(commdata);
	return NULL;
}

/* 销毁一个struct comm_data的结构体 */
void commdata_destroy(struct comm_data *commdata)
{
	if (likely(commdata)) {
		commqueue_destroy(&commdata->recv_queue);
		commqueue_destroy(&commdata->send_queue);
		
		commcache_free(&commdata->recv_buff);
		commcache_free(&commdata->send_buff);

		del_epoll(commdata->commctx->epfd, commdata->portinfo.fd, 0);
		commdata->commctx->watchcnt --;
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
