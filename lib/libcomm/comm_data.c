/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/23.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_data.h"

#define NODESIZE	sizeof(intptr_t)	/* 队列里面保存的一个节点的大小 */
#define	QUEUENODES	1024			/* 队列里面保存的节点总个数 */
#define	MAXDISPOSEDATA	5			/* 最多联系解析打包的数据次数 */

static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *packager);

bool commdata_init(struct connfd_info **connfd, struct comm_tcp* commtcp,  struct cbinfo*  finishedcb)
{
	assert(commtcp && commtcp && commtcp->fd > 0);

	New(*connfd);
	if (unlikely(*connfd == NULL)) {
		goto error;
	}
	if (unlikely(!commqueue_init(&(*connfd)->send_queue, NODESIZE, QUEUENODES, free_commmsg))) {
		goto error;
	}
	if (unlikely(!commlock_init(&(*connfd)->sendlock))) {
		goto error;
	}
	commlist_init(&(*connfd)->send_list, free_commmsg);
	commcache_init(&(*connfd)->send_cache);
	commcache_init(&(*connfd)->recv_cache);
	mfptp_parse_init(&(*connfd)->parser, &(*connfd)->recv_cache.buffer, &(*connfd)->recv_cache.size);
	mfptp_package_init(&(*connfd)->packager, &(*connfd)->send_cache.buffer, &(*connfd)->send_cache.size);

	memcpy(&(*connfd)->commtcp, commtcp, sizeof(*commtcp));
	if (finishedcb) {
		memcpy(&(*connfd)->finishedcb, finishedcb, sizeof(*finishedcb));
	}
	return true;

error:
	commlock_destroy(&(*connfd)->sendlock);
	commqueue_destroy(&(*connfd)->send_queue);
	commcache_free(&(*connfd)->recv_cache);
	commcache_free(&(*connfd)->send_cache);
	commlist_destroy(&(*connfd)->send_list, COMMMSG_OFFSET);
	Free(*connfd);
	return false;
}

void commdata_destroy(struct connfd_info *connfd)
{
	if (likely(connfd)) {
		commlock_destroy(&connfd->sendlock);
		commqueue_destroy(&connfd->send_queue);
		commcache_free(&connfd->recv_cache);
		commcache_free(&connfd->send_cache);
		commlist_destroy(&connfd->send_list, COMMMSG_OFFSET);
		close(connfd->commtcp.fd);
		connfd->commtcp.fd = -1;
		Free(connfd);
	}

	return ;
}

bool commdata_package(struct connfd_info *connfd, struct comm_event *commevent)
{
	assert(connfd && commevent && commevent->init);

	int			size = -1;
	int			counter = 0;
	bool			flag = false;
	struct comm_message*	message = NULL;

	while (counter < MAXDISPOSEDATA) {

		commlock_lock(&connfd->sendlock);
		if (unlikely(!commqueue_pull(&connfd->send_queue, (void*)&message))) {
			struct comm_list *list = NULL;
			if (commlist_pull(&connfd->send_list, &list)) {
				message = (struct comm_message*)get_container_addr(list, COMMMSG_OFFSET);
			} else {
				/* 没有数据 则直接返回 */
				commlock_unlock(&connfd->sendlock);
				del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PACKAGE); /* 没有数据需要进行打包	*/
				return flag;
			}
		}
		commlock_unlock(&connfd->sendlock);

		size = mfptp_check_memory(connfd->send_cache.capacity - connfd->send_cache.size, message->package.frames, message->package.dsize);
		if (size > 0) {
			/* 检测到内存不够 则增加内存*/
			if (unlikely(!commcache_expend(&connfd->send_cache, size))) {
				/* 增加内存失败 则将数据再次存起来 停止打包 */
				commlock_lock(&connfd->sendlock);
				if (unlikely(!commqueue_push(&connfd->send_queue, (void*)&message))) {
					commlist_push(&connfd->send_list, &message->list);
				}
				commlock_unlock(&connfd->sendlock);
				add_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PACKAGE); /* 还有数据需要进行打包 */
				return flag;
			}
		}
		mfptp_fill_package(&connfd->packager, message->package.frame_offset, message->package.frame_size, message->package.frames_of_package, message->package.packages);
		size = mfptp_package(&connfd->packager, message->content, message->config, message->socket_type);
		if (size > 0 && connfd->packager.ms.error == MFPTP_OK) {
			connfd->send_cache.end += size;
			//log("package successed\n");
			flag = true;	/* 只要有一个打包成功，都会返回true，因为有数据可以进行发送 */
		} else {
		//	log("package failed\n");
			connfd->packager.ms.error = MFPTP_OK;
		}
		free_commmsg(message);	/* 打包失败也会直接放弃这个有问题的包 */
		counter ++;
	}
	if (connfd->send_queue.nodes > 0 || connfd->send_list.nodes > 0) {
		add_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PACKAGE); /* 节点数大于0 说明还有数据需要进行打包 */
	} else {
		del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PACKAGE); /* 节点数小于0 说明没有数据需要进行打包	*/
	}
	return flag;
}

bool commdata_parse(struct connfd_info *connfd, struct comm_event *commevent)
{
	assert(connfd && commevent && commevent->init);

	bool			flag = false;
	int			size = 0;	/* 成功解析数据的字节数 */
	int			counter = 0;
	struct comm_message	*message = NULL;

	while (counter < MAXDISPOSEDATA) {
		if (connfd->recv_cache.size > 0) {
			size = mfptp_parse(&connfd->parser);
			if ((size > 0) && connfd->parser.ms.error == MFPTP_OK && connfd->parser.ms.step == MFPTP_PARSE_OVER) {	/* 成功解析了一个连续的包 */
				if (likely(new_commmsg(&message, connfd->parser.package.dsize))) {
					message->fd = connfd->commtcp.fd;
					memcpy(message->content, &connfd->parser.ms.cache.buffer[connfd->parser.ms.cache.start], connfd->parser.package.dsize);
					_fill_message_package(message, &connfd->parser);
					connfd->parser.ms.cache.start += connfd->parser.package.dsize;
					connfd->parser.ms.cache.size -= connfd->parser.package.dsize;
					connfd->recv_cache.start += size;
					connfd->recv_cache.size -= size;
					commcache_clean(&connfd->recv_cache);
					commcache_clean(&connfd->parser.ms.cache);

					commlock_lock(&commevent->commctx->recvlock);
					if (unlikely(!commqueue_push(&commevent->commctx->recvqueue, (void*)&message))) {
						/* 队列已满，则放入链表中 */
						if (commlist_push(&commevent->commctx->recvlist, &message->list)) {
							if (commevent->commctx->recvqueue.readable == 0) {			/* 为0代表有线程在等待可读 */
								/* 唤醒在commctx->recv_queue.readable上等待的线程并设置其为1 */
								commlock_wake(&commevent->commctx->recvlock, &commevent->commctx->recvqueue.readable, 1, true);
							}
						}
					} else if (commevent->commctx->recvqueue.readable == 0) {	/* 为0不可读，代表有线程在等待可读 */
						/* 唤醒在commctx->recv_queue.readable上等待的线程并设置其为1 */
						commlock_wake(&commevent->commctx->recvlock, &commevent->commctx->recvqueue.readable, 1, true);
					}
					commlock_unlock(&commevent->commctx->recvlock);
					flag = true;	/* 只要有一个数据数据解析成功便返回真 */
					//log("parse successed and push the data\n");

				} 
			} else if (connfd->parser.ms.error == MFPTP_DATA_TOOFEW) {
				connfd->parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
				del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PARSE);
				return flag;
			} else if (connfd->parser.ms.error != MFPTP_DATA_TOOFEW) {
				/* 解析出错 抛弃已解析的错误数据 继续解析后面的数据 */
				connfd->recv_cache.start += size;
				connfd->recv_cache.size -= size;
				commcache_clean(&connfd->recv_cache);
				connfd->parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
				//log("parse failed\n");
			}
		} else  {
			/* 没有数据需要进行解析， 则去查找此fd是否存在在remainfd里面，存在则删除 */
			del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PARSE);
			return flag;
		}
		counter ++;
	}

	if (connfd->recv_cache.size > 0) {
		add_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PARSE);	/* 数据未解析完，则添加到remainfd里面 */
	} else {
		del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PARSE);	/* 数据已解析完，则将remainfd里面的此fd删除 */
	}

	return flag;
}

/* 填充message结构体 */ 
static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *parser) 
{
	assert(message && parser);
	int k = 0;
	int pckidx = 0;		/* 包的索引 */
	int frmidx = 0;		/* 帧的缩影 */
	int frames = 0;		/* 总帧数 */
	const struct mfptp_package_info* package = &parser->package;
	const struct mfptp_header_info* header = &parser->header;
	for (pckidx = 0; pckidx < package->packages; pckidx++) {
		for (frmidx = 0; frmidx < package->frame[pckidx].frames; frmidx++, k++) {
			message->package.frame_size[k] = package->frame[pckidx].frame_size[frmidx];
			message->package.frame_offset[k] = package->frame[pckidx].frame_offset[frmidx] - parser->ms.cache.start;	/* 每段偏移都是从此缓冲区的*/
		}
		message->package.frames_of_package[pckidx] = package->frame[pckidx].frames;
		frames += package->frame[pckidx].frames;
	}
	message->package.packages = package->packages;
	message->package.frames = frames;
	message->package.dsize = package->dsize;

	message->config = header->compression | header->encryption;
	message->socket_type = header->socket_type;
}
