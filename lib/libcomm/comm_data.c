/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/23.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_data.h"

#define NODESIZE	sizeof(intptr_t)	/* 队列里面保存的一个节点的大小 */
#define	QUEUENODES	1024			/* 队列里面保存的节点总个数 */

static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *packager);

/* 初始化一个fd的数据结构体，并将此fd添加到epoll的监控中 */
bool commdata_init(struct comm_data **commdata, struct comm_tcp* commtcp,  struct cbinfo*  finishedcb)
{
	assert(commtcp && commtcp && commtcp->fd > 0);

	New(*commdata);
	if (unlikely(*commdata == NULL)) {
		goto error;
	}

#if 0
	/* 监听套接字的时间太晚，可能会出现事件已发生，但是epoll却没监听到 */
	if (unlikely(!commepoll_add(commepoll, commtcp->fd, EPOLLIN | EPOLLOUT | EPOLLET))) {
		goto error;
	}
	retval = commqueue_init(&commdata->recv_queue, QUEUE_CAPACITY, nodesize, free_commmsg);
	if (unlikely(!retval)) {
		goto error;
	}
#endif
	if (unlikely(!commqueue_init(&(*commdata)->send_queue, NODESIZE, QUEUENODES, free_commmsg))) {
		goto error;
	}
	if (unlikely(!commlock_init(&(*commdata)->sendlock))) {
		goto error;
	}

	//commdata->commctx = commctx;
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
	//commqueue_destroy(&(*commdata)->recv_queue);
	commqueue_destroy(&(*commdata)->send_queue);
	//commepoll_destroy(commepoll);
	
	commcache_free(&(*commdata)->recv_cache);
	commcache_free(&(*commdata)->send_cache);
	Free(*commdata);
	return false;
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

	//	commepoll_del(&commdata->commctx->commepoll, commdata->commtcp.fd, -1);
		Free(commdata);
	}

	return ;
}

bool commdata_package(struct comm_data *commdata, struct comm_event *commevent)
{
	bool			flag = false;
	bool			block = false;
	int			packsize = 0; /* 打包之后的数据大小 */
	struct comm_list*	list = NULL;
	struct comm_context*	commctx = commevent->commctx;
	struct comm_message*	message = NULL;

	commlock_lock(&commdata->sendlock);
	if (likely(commqueue_pull(&commdata->send_queue, (void*)&message))) {
		flag = true;
	} else if (likely(commlist_pull(&commctx->msghead, &list))) {
		/* 无数据可打包的时候 应该直接退出 */
		message = (struct comm_message*)get_container_addr(list, COMMMSG_OFFSET);
		if (message->fd == commdata->commtcp.fd) {
			flag = true;
		} else {
			flag = false;
			commdata = (struct comm_data*)commevent->data[message->fd];
		}
	}
	commlock_unlock(&commdata->sendlock);

	if (likely(message)) {
		int size = 0;
		bool pckflag = true;
		size = mfptp_check_memory(commdata->send_cache.capacity - commdata->send_cache.size, message->package.frames, message->package.dsize);
		if (size > 0) {
			/* 检测到内存不够 则增加内存*/
			if (unlikely(!commcache_expend(&commdata->send_cache, size))) {
				/* 增加内存失败 */
				pckflag = false;
			}
		}
		if (likely(pckflag)) {
			mfptp_fill_package(&commdata->packager, message->package.frame_offset, message->package.frame_size, message->package.frames_of_package, message->package.packages);
			packsize = mfptp_package(&commdata->packager, message->content, message->config, message->socket_type);
			if (packsize > 0 && commdata->packager.ms.error == MFPTP_OK) {
				commdata->send_cache.end += packsize;
				log("package successed\n");
			} else {
				flag = false;
				log("package failed\n");
				commdata->packager.ms.error = MFPTP_OK;
			}
		}
		free_commmsg(message);
	}

	return flag;
}

bool commdata_parse(struct comm_data *commdata, struct comm_event *commevent)
{
	assert(commdata);
	bool			flag = false;
	bool			block = false;
	int			size = 0;	/* 成功解析数据的字节数 */
	struct comm_context	*commctx = commevent->commctx;
	struct comm_message	*message = NULL;

	/* 有数据进行解析的时候才去进行解析并将数据全部解析完毕 */
	while (commdata->recv_cache.size > 0) {
		flag = false;
		size = mfptp_parse(&commdata->parser);
		if (likely(size > 0 && commdata->parser.ms.error == MFPTP_OK && commdata->parser.ms.step == MFPTP_PARSE_OVER)) {	/* 成功解析了一个连续的包 */
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
				commlock_lock(&commctx->recvlock);
				if (likely(commqueue_push(&commctx->recvqueue, (void*)&message))) {
					if (unlikely(!commctx->recvqueue.readable)) {			/* 为0代表有线程在等待可读 */
						/* 唤醒在commctx->recv_queue.readable上等待的线程并设置其为1 */
						commlock_wake(&commctx->recvlock, &commctx->recvqueue.readable, 1, true);
					}
					flag = true;
				} else {
					/* 解析数据的时候队列已满，另作处理 不进行堵塞等待用户取数据 */
					//flag = false;
					flag = true;
				}
				commlock_unlock(&commctx->recvlock);
				if (!flag) {
					break ;
				}
				log("parse successed\n");
			} 
		} else if (commdata->parser.ms.error != MFPTP_DATA_TOOFEW) {
			/* 解析出错 抛弃已解析的错误数据  */
			flag = false;
			commdata->recv_cache.start += size;
			commdata->recv_cache.size -= size;
			commcache_clean(&commdata->recv_cache);
			log("parse failed\n");
			commdata->parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
			break ;
		} else if (commdata->parser.ms.error == MFPTP_DATA_TOOFEW) {
			commdata->parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
			break ;
		}
	}
#if 0
	/* 解析成功了 */
	if (flag ) {
		commdata->parser.ms.dosize = 0;
		memset(&commdata->parser.package, 0, sizeof(commdata->parser.package));
		commcache_clean(&commdata->recv_cache);
		commcache_clean(&commdata->parser.ms.cache);
	}
#endif
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
