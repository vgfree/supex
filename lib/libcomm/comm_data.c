/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/23.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_data.h"

#define NODESIZE        sizeof(intptr_t)	/* 队列里面保存的一个节点的大小 */
#define QUEUENODES      1024			/* 队列里面保存的节点总个数 */
#define MAXDISPOSEDATA  5			/* 最多联系解析打包的数据次数 */
#define COMM_READ_MIOU  1024*1024		/* 一次性最多读取数据的字节数 */
#define	CONNECTVALUE	1000*20			/* 当服务器端断开之后,过多长时间尝试第一次连接服务器[单位 ms]*/
#define	CONNECTINTERVAL 1000*60			/* 当服务器断开之后，每间隔多长时间去尝试连接直到连接上服务器[单位 ms]*/

/* 删除一个fd的相关信息以及关闭一个fd */
#define DELETEFD(commevent, fd, flag)				\
	({      commdata_del(commevent, fd);			\
		del_remainfd(&commevent->remainfd, fd, flag);	\
		close(fd);					\
	 })

static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *packager);

static void  _timer_event(struct comm_timer *commtimer, struct comm_list *timerhead, void *usr);

bool commdata_init(struct connfd_info **connfd, struct comm_event *commevent, struct comm_tcp *commtcp, struct cbinfo *finishedcb)
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

	if (unlikely(!mfptp_parse_init(&(*connfd)->parser, &(*connfd)->recv_cache.buffer, &(*connfd)->recv_cache.size))) {
		goto error;
	}

	if (unlikely(!mfptp_package_init(&(*connfd)->packager, &(*connfd)->send_cache.buffer, &(*connfd)->send_cache.size))) {
		goto error;
	}
	
	if (commtcp->connattr == CONNECT_ANYWAY) {

		(*connfd)->commtimer = commtimer_create(CONNECTVALUE, CONNECTINTERVAL, _timer_event, (void*)*connfd);
		if (unlikely((*connfd)->commtimer == NULL)) {
			goto error;
		}
	}

	(*connfd)->commevent = commevent;
	commlist_init(&(*connfd)->send_list, free_commmsg);
	commcache_init(&(*connfd)->send_cache);
	commcache_init(&(*connfd)->recv_cache);
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
	mfptp_parse_destroy(&(*connfd)->parser);
	mfptp_package_destroy(&(*connfd)->packager);
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
		commtimer_destroy(connfd->commtimer);
		mfptp_parse_destroy(&connfd->parser);
		mfptp_package_destroy(&connfd->packager);
		close(connfd->commtcp.fd);
		connfd->commtcp.fd = -1;
		Free(connfd);
	}
}

/* @返回值：true:此fd已被处理完毕，移除了remainfd的结构体 false：此fd还没处理完毕，还保存在remainfd里面，下次轮寻的时候处理 */
bool commdata_recv(struct connfd_info *connfd, struct comm_event *commevent, int fd)
{
	assert(commevent && commevent->init);

	int     bytes = 0;
	char    buff[COMM_READ_MIOU] = {};

	if (connfd != NULL) {
		connfd->commtcp.stat = FD_READ;
		while ((bytes = read(connfd->commtcp.fd, buff, COMM_READ_MIOU)) > 0) {
			commcache_append(&connfd->recv_cache, buff, bytes);
			if (bytes < COMM_READ_MIOU) {		/* 数据已经读取完毕 */
				break;
			}
		}

		if (bytes == 0) {
			connfd->commtcp.stat = FD_CLOSE;
		} else if (bytes < 0) {
			if (errno == EINTR) {						/* 读操作被中断，则返回false，下次轮询接着处理此fd */
				return false;
			} else if ((errno != EAGAIN) || (errno != EWOULDBLOCK)) {	/* 发生致命错误，则删除此fd的相关信息 */
				DELETEFD(commevent, connfd->commtcp.fd, REMAINFD_READ);
				return true;
			}
		}

		if (connfd->finishedcb.callback) {
			connfd->finishedcb.callback(commevent->commctx, &connfd->commtcp, connfd->finishedcb.usr);
		}

		if (connfd->commtcp.stat == FD_CLOSE) {
			if (connfd->commtcp.connattr == CONNECT_ANYWAY) {
				del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_READ);
				del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_WRITE);
				del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PARSE);
				del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PACKAGE);
				commepoll_del(&commevent->commctx->commepoll, connfd->commtcp.fd, 0);
				commevent->connfd[connfd->commtcp.fd] = 0;
				commevent->connfdcnt--;
				close(connfd->commtcp.fd);
				commtimer_start(connfd->commtimer, &commevent->commctx->timerhead);
				log("%d closed and start commtimer\n", connfd->commtcp.fd);
			} else {
				DELETEFD(commevent, connfd->commtcp.fd, REMAINFD_READ);
				log("%d closed\n", connfd->commtcp.fd);
			}
		} else {
			/* 读事件成功处理完毕，则添加解析事件并将此fd从读事件数组里移除 */
			add_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PARSE);
			del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_READ);
			//	log("read data successed:%d\n", connfd->commtcp.fd);
		}

		return true;
	}
	del_remainfd(&commevent->remainfd, fd, REMAINFD_READ);
	return true;
}

/* @返回值：true:此fd已被处理完毕，移除了remainfd的结构体 false：此fd还没处理完毕，还保存在remainfd里面，下次轮寻的时候处理 */
bool commdata_send(struct connfd_info *connfd, struct comm_event *commevent, int fd)
{
	assert(commevent && commevent->init);
	int     bytes = 0;
	bool    flag = true;

	if (connfd != NULL) {
		connfd->commtcp.stat = FD_WRITE;
		if (connfd->send_cache.size > 0) {
			bytes = write(connfd->commtcp.fd, &connfd->send_cache.buffer[connfd->send_cache.start], connfd->send_cache.size);

			if (bytes > 0) {
				if (unlikely(bytes < connfd->send_cache.size)) {
					/* 数据没有一次性发送完毕，说明系统缓冲区已满，返回false，下次继续发送剩下的数据 */
					flag = false;
				}
				connfd->send_cache.start += bytes;
				connfd->send_cache.size -= bytes;
				commcache_clean(&connfd->send_cache);
			} else {
				if (errno == EINTR) {
					log("write EINTR\n");
					flag = false;	/* 被打断，则下次继续处理 */
				} else if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
					log("write cache full\n");
					flag = true;	/* 缓冲区已满，则从remainfd里面删除此fd，等待此fd的epoll写事件被触发再继续发送数据 */
				} else {
					/* 出现其他的致命错误，返回true，将此fd的相关信息删除 */
					log("write dead wrong\n");
					DELETEFD(commevent, connfd->commtcp.fd, REMAINFD_WRITE);
					return true;
				}
			}
			if (flag && connfd->finishedcb.callback) {		/* 写事件正常执行成功 */
				connfd->finishedcb.callback(commevent->commctx, &connfd->commtcp, connfd->finishedcb.usr);
			}
		}
		if (flag) {
			//log("write deal over\n");
			del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_WRITE);
		} else {
			//log("write not over\n");
		}
		return flag;
	}
	del_remainfd(&commevent->remainfd, fd, REMAINFD_WRITE);	/* 此描述符已被删除或不可写，则移除此描述符 */
	return flag;
}

bool commdata_package(struct connfd_info *connfd, struct comm_event *commevent, int fd)
{
	assert(commevent && commevent->init);

	int                     size = -1;
	struct comm_message     *message = NULL;

	if (connfd != NULL) {
		commlock_lock(&connfd->sendlock);
#if 0
		struct comm_list *list = NULL;

		if (commlist_pull(&connfd->send_list, &list)) {
			message = (struct comm_message *)get_container_addr(list, COMMMSG_OFFSET);
		} else {
			/* 没有数据 则直接返回 */
			commlock_unlock(&connfd->sendlock);
			del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PACKAGE);	/* 没有数据需要进行打包	*/
			return true;
		}
#endif

		if (unlikely(!commqueue_pull(&connfd->send_queue, (void *)&message))) {
			struct comm_list *list = NULL;

			if (commlist_pull(&connfd->send_list, &list)) {
				message = (struct comm_message *)get_container_addr(list, COMMMSG_OFFSET);
			} else {
				/* 没有数据 则直接返回 */
				commlock_unlock(&connfd->sendlock);
				del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PACKAGE);	/* 没有数据需要进行打包	*/
				return true;
			}
		}

		commlock_unlock(&connfd->sendlock);

		size = mfptp_check_memory(connfd->send_cache.capacity - connfd->send_cache.size, message->package.frames, message->package.dsize);

		if (size > 0) {
			/* 检测到内存不够 则增加内存*/
			if (unlikely(!commcache_expend(&connfd->send_cache, size))) {
				/* 增加内存失败 则将数据再次存起来 停止打包 */
				commlock_lock(&connfd->sendlock);

				if (unlikely(!commqueue_push(&connfd->send_queue, (void *)&message))) {
					commlist_push(&connfd->send_list, &message->list);
				}
				commlock_unlock(&connfd->sendlock);
				return false;
			}
		}

		mfptp_fill_package(&connfd->packager, message->package.frame_offset, message->package.frame_size, message->package.frames_of_package, message->package.packages);
		size = mfptp_package(&connfd->packager, message->content, message->config, message->socket_type);

		if ((size > 0) && (connfd->packager.ms.error == MFPTP_OK)) {
			connfd->send_cache.end += size;
			add_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_WRITE);
			log("package successed\n");
		} else {
			log("package failed\n");
			connfd->packager.ms.error = MFPTP_OK;
		}

		free_commmsg(message);	/* 打包失败也会直接放弃这个有问题的包 */

		if ((connfd->send_queue.nodes == 0) && (connfd->send_list.nodes == 0)) {
			del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PACKAGE);	/* 节点数等于0 说明没有数据需要进行打包 打包事件处理完毕 */
			return true;
		} else {
			return false;
		}
	}

	del_remainfd(&commevent->remainfd, fd, REMAINFD_PACKAGE);
	return true;
}

bool commdata_parse(struct connfd_info *connfd, struct comm_event *commevent, int fd)
{
	assert(commevent && commevent->init);

	bool                    flag = false;
	int                     size = 0;	/* 成功解析数据的字节数 */
	struct comm_message     *message = NULL;

	if (connfd != NULL) {
		if (connfd->recv_cache.size > 0) {
			do {
				size = mfptp_parse(&connfd->parser);
				if ((size > 0) && (connfd->parser.ms.step == MFPTP_PARSE_OVER)) {	/* 成功解析了一个连续的包 */
					flag = true;
					switch (connfd->parser.header.socket_type) {
						case HEARTBEAT_METHOD:	/* 心跳包，则直接丢弃 */
							break ;
						default:
							if (new_commmsg(&message, connfd->parser.bodyer.dsize)) {
								message->fd = connfd->commtcp.fd;
								memcpy(message->content, &connfd->parser.ms.cache.buffer[connfd->parser.ms.cache.start], connfd->parser.bodyer.dsize);
								_fill_message_package(message, &connfd->parser);
								connfd->recv_cache.start += size;
								connfd->recv_cache.size -= size;
								commcache_clean(&connfd->recv_cache);

								commlock_lock(&commevent->commctx->recvlock);

								// commlist_push(&commevent->commctx->recvlist, &message->list);
								if (unlikely(!commqueue_push(&commevent->commctx->recvqueue, (void *)&message))) {
									/* 队列已满，则放入链表中 */
									if (unlikely(!commlist_push(&commevent->commctx->recvlist, &message->list))) {
										flag = false;
									}
								}
								if (flag && (commevent->commctx->recvqueue.readable == 0)) {		/* 为0不可读，代表有线程在等待可读 */
									/* 唤醒在commctx->recv_queue.readable上等待的线程并设置其为1 */
									commlock_wake(&commevent->commctx->recvlock, &commevent->commctx->recvqueue.readable, 1, true);
								}
								commlock_unlock(&commevent->commctx->recvlock);
								// log("parse successed and push the data\n");
							} else {
								/* 分配内存失败 数据未处理 */
							}
					}
				} else if (connfd->parser.ms.error == MFPTP_DATA_TOOFEW) {
					connfd->parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
					del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PARSE);
					return true;
				} else {
					/* 解析出错 抛弃已解析的错误数据 继续解析后面的数据 */
					connfd->recv_cache.start += size;
					connfd->recv_cache.size -= size;
					commcache_clean(&connfd->recv_cache);
					connfd->parser.ms.error = MFPTP_OK;	/* 重新恢复正常值 */
					log("parse failed\n");
				}
				if (connfd->recv_cache.size == 0) {
					del_remainfd(&commevent->remainfd, connfd->commtcp.fd, REMAINFD_PARSE);	/* 数据已解析完，则将remainfd里面的此fd删除 */
					return true;
				} else if (flag) {
					return false;
				}
			}while(1);
		}
	}
	del_remainfd(&commevent->remainfd, fd, REMAINFD_PARSE);
	return true;
}

bool commdata_add(struct comm_event *commevent, struct comm_tcp *commtcp, struct cbinfo *finishedcb)
{
	assert(commevent && commevent->init && commtcp && commtcp->fd > 0);

	if ((commtcp->type == COMM_CONNECT) || (commtcp->type == COMM_ACCEPT)) {
		struct connfd_info *connfd = NULL;

		if (commepoll_add(&commevent->commctx->commepoll, commtcp->fd, EPOLLIN | EPOLLOUT | EPOLLET)) {
			if (commdata_init(&connfd, commevent, commtcp, finishedcb)) {
				commevent->connfd[commtcp->fd] = connfd;
				commevent->connfdcnt++;
				return true;
			}
		}
	} else {
		if (commepoll_add(&commevent->commctx->commepoll, commtcp->fd, EPOLLIN | EPOLLET)) {
			memcpy(&commevent->bindfd[commevent->bindfdcnt].commtcp, commtcp, sizeof(*commtcp));

			if (finishedcb) {
				memcpy(&commevent->bindfd[commevent->bindfdcnt].finishedcb, finishedcb, sizeof(*finishedcb));
			}

			commevent->bindfdcnt++;
			return true;
		}
	}

	return false;
}

void commdata_del(struct comm_event *commevent, int fd)
{
	assert(commevent && commevent->init && fd > 0);
	int                     fdidx = -1;
	struct connfd_info      *connfd = NULL;
	commepoll_del(&commevent->commctx->commepoll, fd, -1);

	if ((connfd = commevent->connfd[fd])) {
		commdata_destroy(connfd);
		commevent->connfd[fd] = 0;
		commevent->connfdcnt--;
	} else if ((fdidx = gain_bindfd_fdidx(commevent, fd)) > -1) {
		if ((fdidx + 1) != commevent->bindfdcnt) {
			/* 如果删除的fd不是最后一个fd，则将后面的fd数据往前拷贝 */
			int size = (sizeof(struct comm_tcp)) * (commevent->bindfdcnt - fdidx - 2);
			memmove(&commevent->bindfd[fdidx].commtcp, &commevent->bindfd[fdidx + 1].commtcp, size);
			memmove(&commevent->bindfd[fdidx].finishedcb, &commevent->bindfd[fdidx + 1].finishedcb, size);
		}

		commevent->bindfdcnt--;
	}
}

/* 填充message结构体 */
static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *parser)
{
	assert(message && parser);
	int                             k = 0;
	int                             pckidx = 0;	/* 包的索引 */
	int                             frmidx = 0;	/* 帧的索引 */
	int                             frames = 0;	/* 总帧数 */
	const struct mfptp_bodyer_info  *bodyer = &parser->bodyer;
	const struct mfptp_header_info  *header = &parser->header;

	for (pckidx = 0; pckidx < bodyer->packages; pckidx++) {
		for (frmidx = 0; frmidx < bodyer->package[pckidx].frames; frmidx++, k++) {
			message->package.frame_size[k] = bodyer->package[pckidx].frame[frmidx].frame_size;
			message->package.frame_offset[k] = bodyer->package[pckidx].frame[frmidx].frame_offset - parser->ms.cache.start;
		}

		message->package.frames_of_package[pckidx] = bodyer->package[pckidx].frames;
		frames += bodyer->package[pckidx].frames;
	}

	message->package.packages = bodyer->packages;
	message->package.frames = frames;
	message->package.dsize = bodyer->dsize;

	message->config = header->compression | header->encryption;
	message->socket_type = header->socket_type;
}

static void  _timer_event(struct comm_timer *commtimer, struct comm_list *timerhead, void *usr)
{
	assert(usr);

	char service[64] = {};
	char host[MAXIPADDRLEN] = {};
	struct connfd_info *connfd = (struct connfd_info*)usr;

	sprintf(service, "%d", connfd->commtcp.peerport);
	memcpy(host, connfd->commtcp.peeraddr, strlen(connfd->commtcp.peeraddr));

	log("deal with connect timer\n");
	if (socket_connect(&connfd->commtcp, host, service, 0, CONNECT_ANYWAY)) {
		/* 连接成功， 则停止计时器 */
		commtimer_stop(commtimer, timerhead);
		if (commepoll_add(&connfd->commevent->commctx->commepoll, connfd->commtcp.fd, EPOLLIN | EPOLLOUT | EPOLLET)) {
			connfd->commevent->connfd[connfd->commtcp.fd] = connfd;
			connfd->commevent->connfdcnt ++;
			if (connfd->finishedcb.callback) {
				connfd->finishedcb.callback(connfd->commevent->commctx, &connfd->commtcp, connfd->finishedcb.usr);
			}
			log("timer event connect fd :%d, stop timer\n", connfd->commtcp.fd);
		} else {
			close(connfd->commtcp.fd);
			commdata_destroy(connfd);
			commepoll_del(&connfd->commevent->commctx->commepoll, connfd->commtcp.fd, 0);
		}
	}
	return ;
}
