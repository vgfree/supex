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
#if 0
	retval = commqueue_init(&commdata->recv_queue, QUEUE_CAPACITY, nodesize, free_commmsg);
	if (unlikely(!retval)) {
		goto error;
	}
#endif
	retval = commqueue_init(&commdata->send_queue, QUEUE_CAPACITY, nodesize, free_commmsg);
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

	if (!mfptp_parse_init(&commdata->parser, &commdata->recv_buff.cache, &commdata->recv_buff.size)) {
		goto error;
	}
	mfptp_package_init(&commdata->packager, commdata->send_buff.cache, &commdata->send_buff.size);

	commdata->commctx = commctx;
	memcpy(&commdata->portinfo, portinfo, sizeof(*portinfo));
	if (finishedcb) {
		memcpy(&commdata->finishedcb, finishedcb, sizeof(*finishedcb));
	}
	return commdata;

error:
	commlock_destroy(&commdata->sendlock);
	//commqueue_destroy(&commdata->recv_queue);
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
	//	commqueue_destroy(&commdata->recv_queue);
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
	memset(listenfd->portinfo, 0, sizeof(listenfd->portinfo));
	return ;
}

/* 销毁struct listenfd结构体 */
inline void listenfd_destroy(struct listenfd *listenfd)
{
	assert(listenfd);
	int i = 0;
	for (i = 0; i < listenfd->counter; i++) {
		memset(&listenfd->portinfo[i], 0, sizeof(listenfd->portinfo[i]));
		memset(&listenfd->finishedcb[i], 0, sizeof(listenfd->finishedcb[i]));
		commepoll_del(&listenfd->commctx->commepoll, listenfd->fd[i], -1);	
		listenfd->fd[i] = -1;
	}
	listenfd->counter = 0;
	return ;
}

/* 添加一个监听fd */
inline bool add_listenfd(struct listenfd *listenfd, struct cbinfo *finishedcb, struct portinfo *portinfo, int fd)
{
	assert(listenfd && fd > 0);
	memcpy(&listenfd->portinfo[listenfd->counter], portinfo, sizeof(*portinfo));
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
		memset(&listenfd->portinfo[fdidx], 0, sizeof(listenfd->portinfo[fdidx]));
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

inline bool get_portinfo(struct portinfo *portinfo, int fd, int type, int status)
{
	assert(portinfo && portinfo->addr);
	if (unlikely((get_address(fd, portinfo->addr, (size_t)sizeof(portinfo->addr))) == -1)) {
		return false;
	}
	portinfo->port = get_port(fd);
	if (unlikely(portinfo->port ==  -1)) {
		return false;
	}
	portinfo->fd = fd;
	portinfo->type = type;
	portinfo->stat = status;

	return true;
}
