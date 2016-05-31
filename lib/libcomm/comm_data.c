/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/23.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_data.h"

#define NODESIZE	sizeof(intptr_t)	/* 队列里面保存的一个节点的大小 */
#define	QUEUENODES	1024			/* 队列里面保存的节点总个数 */
#define	MAXDISPOSEDATA	5			/* 最多联系解析打包的数据次数 */

static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *packager);

bool commdata_init(struct comm_data **commdata, struct comm_tcp* commtcp,  struct cbinfo*  finishedcb)
{
	assert(commtcp && commtcp && commtcp->fd > 0);

	New(*commdata);
	if (unlikely(*commdata == NULL)) {
		goto error;
	}
	if (unlikely(!commqueue_init(&(*commdata)->send_queue, NODESIZE, QUEUENODES, free_commmsg))) {
		goto error;
	}
	if (unlikely(!commlock_init(&(*commdata)->sendlock))) {
		goto error;
	}
	commlist_init(&(*commdata)->send_list, free_commmsg);
	commcache_init(&(*commdata)->send_cache);
	commcache_init(&(*commdata)->recv_cache);
	mfptp_parse_init(&(*commdata)->parser, &(*commdata)->recv_cache.buffer, &(*commdata)->recv_cache.size);
	mfptp_package_init(&(*commdata)->packager, &(*commdata)->send_cache.buffer, &(*commdata)->send_cache.size);

	memcpy(&(*commdata)->commtcp, commtcp, sizeof(*commtcp));
	if (finishedcb) {
		memcpy(&(*commdata)->finishedcb, finishedcb, sizeof(*finishedcb));
	}
	return true;

error:
	commlock_destroy(&(*commdata)->sendlock);
	commqueue_destroy(&(*commdata)->send_queue);
	commcache_free(&(*commdata)->recv_cache);
	commcache_free(&(*commdata)->send_cache);
	commlist_destroy(&(*commdata)->send_list, COMMMSG_OFFSET);
	Free(*commdata);
	return false;
}

void commdata_destroy(struct comm_data *commdata)
{
	if (likely(commdata)) {
		commlock_destroy(&commdata->sendlock);
		commqueue_destroy(&commdata->send_queue);
		commcache_free(&commdata->recv_cache);
		commcache_free(&commdata->send_cache);
		commlist_destroy(&commdata->send_list, COMMMSG_OFFSET);
		Free(commdata);
	}

	return ;
}

bool commdata_package(struct comm_data *commdata, struct comm_event *commevent)
{
	assert(commdata && commevent && commevent->init);

	int			size = -1;
	int			counter = 0;
	bool			flag = false;
	struct comm_message*	message = NULL;

	while (counter < MAXDISPOSEDATA) {

		commlock_lock(&commdata->sendlock);
		if (unlikely(!commqueue_pull(&commdata->send_queue, (void*)&message))) {
			struct comm_list *list = NULL;
			if (commlist_pull(&commdata->send_list, &list)) {
				message = (struct comm_message*)get_container_addr(list, COMMMSG_OFFSET);
			} else {
				/* 没有数据 则直接返回 */
				commlock_unlock(&commdata->sendlock);
				del_remainfd(&commevent->remainfd, commdata->commtcp.fd, REMAINFD_PACK); /* 没有数据需要进行打包	*/
				return flag;
			}
		}
		commlock_unlock(&commdata->sendlock);

		size = mfptp_check_memory(commdata->send_cache.capacity - commdata->send_cache.size, message->package.frames, message->package.dsize);
		if (size > 0) {
			/* 检测到内存不够 则增加内存*/
			if (unlikely(!commcache_expend(&commdata->send_cache, size))) {
				/* 增加内存失败 则将数据再次存起来 停止打包 */
				commlock_lock(&commdata->sendlock);
				if (unlikely(!commqueue_push(&commdata->send_queue, (void*)&message))) {
					commlist_push(&commdata->send_list, &message->list);
				}
				commlock_unlock(&commdata->sendlock);
				add_remainfd(&commevent->remainfd, commdata->commtcp.fd, REMAINFD_PACK); /* 还有数据需要进行打包 */
				return flag;
			}
		}
		mfptp_fill_package(&commdata->packager, message->package.frame_offset, message->package.frame_size, message->package.frames_of_package, message->package.packages);
		size = mfptp_package(&commdata->packager, message->content, message->config, message->socket_type);
		if (size > 0 && commdata->packager.ms.error == MFPTP_OK) {
			commdata->send_cache.end += size;
			//log("package successed\n");
			flag = true;	/* 只要有一个打包成功，都会返回true，因为有数据可以进行发送 */
		} else {
			log("package failed\n");
			commdata->packager.ms.error = MFPTP_OK;
		}
		free_commmsg(message);	/* 打包失败也会直接放弃这个有问题的包 */
		counter ++;
	}
	if (commdata->send_queue.nodes > 0 || commdata->send_list.nodes > 0) {
		add_remainfd(&commevent->remainfd, commdata->commtcp.fd, REMAINFD_PACK); /* 节点数大于0 说明还有数据需要进行打包 */
	} else {
		del_remainfd(&commevent->remainfd, commdata->commtcp.fd, REMAINFD_PACK); /* 节点数小于0 说明没有数据需要进行打包	*/
	}
	return flag;
}

bool commdata_parse(struct comm_data *commdata, struct comm_event *commevent)
{
	assert(commdata && commevent && commevent->init);

	bool			flag = false;
	int			size = 0;	/* 成功解析数据的字节数 */
	int			counter = 0;
	struct comm_message	*message = NULL;

	while (counter < MAXDISPOSEDATA) {
		if (commdata->recv_cache.size > 0) {
			size = mfptp_parse(&commdata->parser);
			if ((size > 0) && commdata->parser.ms.error == MFPTP_OK && commdata->parser.ms.step == MFPTP_PARSE_OVER) {	/* 成功解析了一个连续的包 */
				if (likely(new_commmsg(&message, commdata->parser.package.dsize))) {
					message->fd = commdata->commtcp.fd;
					memcpy(message->content, &commdata->parser.ms.cache.buffer[commdata->parser.ms.cache.start], commdata->parser.package.dsize);
					_fill_message_package(message, &commdata->parser);
					commdata->parser.ms.cache.start += commdata->parser.package.dsize;
					commdata->parser.ms.cache.size -= commdata->parser.package.dsize;
					commdata->recv_cache.start += size;
					commdata->recv_cache.size -= size;
					commcache_clean(&commdata->recv_cache);
					commcache_clean(&commdata->parser.ms.cache);

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
			} else if (commdata->parser.ms.error == MFPTP_DATA_TOOFEW) {
				commdata->parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
				del_remainfd(&commevent->remainfd, commdata->commtcp.fd, REMAINFD_PARSE);
				return flag;
			} else if (commdata->parser.ms.error != MFPTP_DATA_TOOFEW) {
				/* 解析出错 抛弃已解析的错误数据 继续解析后面的数据 */
				commdata->recv_cache.start += size;
				commdata->recv_cache.size -= size;
				commcache_clean(&commdata->recv_cache);
				commdata->parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
				log("parse failed\n");
			}
		} else  {
			/* 没有数据需要进行解析， 则去查找此fd是否存在在remainfd里面，存在则删除 */
			del_remainfd(&commevent->remainfd, commdata->commtcp.fd, REMAINFD_PARSE);
			return flag;
		}
		counter ++;
	}

	if (commdata->recv_cache.size > 0) {
		add_remainfd(&commevent->remainfd, commdata->commtcp.fd, REMAINFD_PARSE);	/* 数据未解析完，则添加到remainfd里面 */
	} else {
		del_remainfd(&commevent->remainfd, commdata->commtcp.fd, REMAINFD_PARSE);	/* 数据已解析完，则将remainfd里面的此fd删除 */
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
