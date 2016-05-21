/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_structure.h"

/* 初始化一个fd的数据结构体，并将此fd添加到epoll的监控中 */
struct comm_data*  commdata_init(struct comm_context* commctx, struct comm_tcp* commtcp,  struct cbinfo*  finishedcb)
{
	assert(commctx && commtcp);
	int			nodesize = sizeof(intptr_t);
	int			flag = 0;
	bool			retval = false;
	struct comm_data	*commdata = NULL;

	New(commdata);
	if (unlikely(!commctx)) {
		goto error;
	}
#if 0
	retval = commqueue_init(&commdata->recv_queue, QUEUE_CAPACITY, nodesize, free_commmsg);
	if (unlikely(!retval)) {
		goto error;
	}
#endif
	retval = commqueue_init(&commdata->send_queue, nodesize, QUEUE_CAPACITY, free_commmsg);
	if (unlikely(!retval)) {
		goto error;
	}
	retval = commlock_init(&commdata->sendlock);
	if( unlikely(!retval)) {
		goto error;
	}

	if (commtcp->type == COMM_BIND) {
		flag = EPOLLIN | EPOLLET;
	} else {
		flag = EPOLLIN | EPOLLOUT | EPOLLET;
	}

	/* 监听套接字的时间太晚，可能会出现事件已发生，但是epoll却没监听到 */
	retval = commepoll_add(&commctx->commepoll, commtcp->fd, flag);
	if( unlikely(!retval) ){
		goto error;
	}

	commdata->commctx = commctx;
	commcache_init(&commdata->send_cache);
	commcache_init(&commdata->recv_cache);
	mfptp_parse_init(&commdata->parser, &commdata->recv_cache.buffer, &commdata->recv_cache.size);
	mfptp_package_init(&commdata->packager, &commdata->send_cache.buffer, &commdata->send_cache.size);

	memcpy(&commdata->commtcp, commtcp, sizeof(*commtcp));
	if (finishedcb) {
		memcpy(&commdata->finishedcb, finishedcb, sizeof(*finishedcb));
	}
	return commdata;

error:
	commlock_destroy(&commdata->sendlock);
	//commqueue_destroy(&commdata->recv_queue);
	commqueue_destroy(&commdata->send_queue);
	commepoll_destroy(&commctx->commepoll);
	
	commcache_free(&commdata->recv_cache);
	commcache_free(&commdata->send_cache);
	Free(commdata);
	return NULL;
}

/* 销毁一个struct comm_data的结构体 */
void commdata_destroy(struct comm_data *commdata)
{
	if (likely(commdata)) {
		commlock_destroy(&commdata->sendlock);
	//	commqueue_destroy(&commdata->recv_queue);
		commqueue_destroy(&commdata->send_queue);
		
		commcache_free(&commdata->recv_cache);
		commcache_free(&commdata->send_cache);

		commepoll_del(&commdata->commctx->commepoll, commdata->commtcp.fd, -1);
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
	destmsg->config = srcmsg->config;
	destmsg->socket_type = srcmsg->socket_type;
	memcpy(&destmsg->package, &srcmsg->package, sizeof(srcmsg->package));
	memcpy(destmsg->content, srcmsg->content, srcmsg->package.dsize);
}

/* 销毁comm_message的结构体 */
inline void free_commmsg(void* arg)
{
	struct comm_message *message = (struct comm_message*)arg;
	if (message) {
		Free(message->content);
		Free(message);
	}
}

/* 初始化struct listenfd的结构体 */
inline void listenfd_init(struct listenfd *listenfd, struct comm_context *commctx)
{
	assert(listenfd);
	int i= 0; 
	for (i = 0; i < LISTEN_SIZE; i++) {
		listenfd->fd[i] = -1;
	}
	listenfd->counter = 0;
	listenfd->commctx = commctx;
	memset(listenfd->finishedcb, 0, sizeof(listenfd->finishedcb));
	memset(listenfd->commtcp, 0, sizeof(listenfd->commtcp));
	return ;
}

/* 销毁struct listenfd结构体 */
inline void listenfd_destroy(struct listenfd *listenfd)
{
	assert(listenfd);
	int i = 0;
	for (i = 0; i < listenfd->counter; i++) {
		memset(&listenfd->commtcp[i], 0, sizeof(listenfd->commtcp[i]));
		memset(&listenfd->finishedcb[i], 0, sizeof(listenfd->finishedcb[i]));
		commepoll_del(&listenfd->commctx->commepoll, listenfd->fd[i], -1);	
		listenfd->fd[i] = -1;
	}
	listenfd->counter = 0;
	return ;
}

/* 添加一个监听fd */
inline bool add_listenfd(struct listenfd *listenfd, struct cbinfo *finishedcb, struct comm_tcp *commtcp, int fd)
{
	assert(listenfd && fd > 0);
	memcpy(&listenfd->commtcp[listenfd->counter], commtcp, sizeof(*commtcp));
	if (finishedcb) {
		memcpy(&listenfd->finishedcb[listenfd->counter], finishedcb, sizeof(*finishedcb));
	}
	listenfd->fd[listenfd->counter] = fd;
	listenfd->counter ++;
	if (likely(commepoll_add(&listenfd->commctx->commepoll, fd, EPOLLIN | EPOLLET))) {
		return true;
	} else {
		return false;
	}
}

/* 删除一个监听fd */
inline bool del_listenfd(struct listenfd *listenfd, int fdidx)
{
	assert(listenfd && listenfd->fd[fdidx] > 0);

	if (likely(commepoll_del(&listenfd->commctx->commepoll, listenfd->fd[fdidx], -1))) {
		listenfd->fd[fdidx] = -1;
		memset(&listenfd->commtcp[fdidx], 0, sizeof(listenfd->commtcp[fdidx]));
		memset(&listenfd->finishedcb[fdidx], 0, sizeof(listenfd->finishedcb[fdidx]));
		listenfd->counter --;
		return true;
	} else {
		return false;
	}
}

/* 检测是否fd是否是监听fd @返回值：-1代表不是，否则返回fd所在数组的下标 */
inline int search_listenfd(struct listenfd *listenfd, int fd)
{
	int fdindex = 0;	/* 代表fd所在数组的下标 */
	for (fdindex = 0; fdindex < listenfd->counter; fdindex++) {
		if (fd == listenfd->fd[fdindex]) {
			return fdindex;
		}
	}
	return -1;
}
