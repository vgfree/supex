/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_confs.h"
#include "comm_dispose_data.h"
#include "comm_dispose_evts.h"

bool commdata_init(struct comm_data *commdata)
{
	/*recv [queue list lock]*/
	if (unlikely(!commlock_init(&commdata->recvlock))) {
		goto error;
	}

	if (unlikely(!commqueue_init(&commdata->recvqueue, sizeof(intptr_t), QUEUE_NODES))) {
		goto error;
	}

	commlist_init(&commdata->recvlist);

	/*send [queue list lock]*/
	if (unlikely(!commlock_init(&commdata->sendlock))) {
		goto error;
	}

	if (unlikely(!commqueue_init(&commdata->sendqueue, sizeof(intptr_t), QUEUE_NODES))) {
		goto error;
	}

	commlist_init(&commdata->sendlist);

	commcache_init(&commdata->send_cache);
	commcache_init(&commdata->recv_cache);

	mfptp_parse_init(&commdata->parser, &commdata->recv_cache.buffer, &commdata->recv_cache.size);
	mfptp_package_init(&commdata->packager, &commdata->send_cache.buffer, &commdata->send_cache.size);

	return true;

error:
	commlock_destroy(&commdata->recvlock);
	commqueue_destroy(&commdata->recvqueue, NULL, NULL);
	commlist_destroy(&commdata->recvlist, NULL, NULL);

	commlock_destroy(&commdata->sendlock);
	commqueue_destroy(&commdata->sendqueue, NULL, NULL);
	commlist_destroy(&commdata->sendlist, NULL, NULL);

	return false;
}

static void travel_msgfcb(const void *data, size_t size, size_t idx, void *usr)
{
	struct comm_message *message = *(struct comm_message **)data;

	commmsg_free(message);
}

void commdata_away(struct comm_data *commdata)
{
	commlock_destroy(&commdata->recvlock);
	commqueue_destroy(&commdata->recvqueue, travel_msgfcb, NULL);
	commlist_destroy(&commdata->recvlist, travel_msgfcb, NULL);

	commlock_destroy(&commdata->sendlock);
	commqueue_destroy(&commdata->sendqueue, travel_msgfcb, NULL);
	commlist_destroy(&commdata->sendlist, travel_msgfcb, NULL);

	commcache_free(&commdata->recv_cache);
	commcache_free(&commdata->send_cache);

	mfptp_parse_destroy(&commdata->parser);
	mfptp_package_destroy(&commdata->packager);
}

int commdata_package(struct comm_data *commdata)
{
	struct comm_message *message = NULL;

	assert(commdata);

	commlock_lock(&commdata->sendlock);
	bool ok = commqueue_pull(&commdata->sendqueue, (void *)&message);

	if (unlikely(!ok)) {
		ok = commlist_pull(&commdata->sendlist, (void *)&message, sizeof(&message));
	}

	commlock_unlock(&commdata->sendlock);

	if (unlikely(!ok)) {
		/* 没有数据 则直接返回 */
		return 0;
	}

	if ((message->flags > 0) && !encrypt_compress_data(message)) {
		/* 加密 压缩失败 则放弃此包 */
		commmsg_free(message);
		return -1;
	}

	int size = mfptp_check_memory(commdata->send_cache.capacity - commdata->send_cache.size,
			message->package.frames, message->package.raw_data.len);

	if (size > 0) {
		/* 检测到内存不够 则增加内存*/
		if (unlikely(!commcache_expand(&commdata->send_cache, size))) {
			/* 增加内存失败 做丢包处理 */
			commmsg_free(message);
			loger("message lost!\n");
			return -1;
		}
	}

	/* 包的信息设置错误或者打包失败也会直接放弃这个有问题的包 */
	mfptp_fill_package(&commdata->packager, message->package.frame_offset, message->package.frame_size,
		message->package.frames_of_package, message->package.packages);
	size = mfptp_package(&commdata->packager, message->package.raw_data.str, message->flags, message->ptype);
	commmsg_free(message);

	if ((size > 0) && (commdata->packager.ms.error == MFPTP_OK)) {
		loger("package successed\n");
		return 1;
	} else {
		loger("package failed\n");
		commdata->packager.ms.error = MFPTP_OK;
		return -1;
	}
}

/* 填充message结构体 */
static void _fill_message_package(struct comm_message *message, const struct mfptp_parser *parser)
{
	assert(message && parser);
	int                             index = 0;
	int                             pckidx = 0;			/* 包的索引 */
	int                             frmidx = 0;			/* 帧的索引 */
	int                             frames = 0;			/* 总帧数 */
	const char                      *data = *parser->ms.data;	/* 待解析的数据缓冲区 */
	const struct mfptp_bodyer_info  *bodyer = &parser->bodyer;
	const struct mfptp_header_info  *header = &parser->header;

	for (pckidx = 0, index = 0; pckidx < bodyer->packages; pckidx++) {
		for (frmidx = 0; frmidx < bodyer->package[pckidx].frames; frmidx++, index++) {
			message->package.frame_offset[index] += message->package.raw_data.len;
			message->package.frame_size[index] = bodyer->package[pckidx].frame[frmidx].frame_size;

			commsds_push_tail(&message->package.raw_data,
				&data[bodyer->package[pckidx].frame[frmidx].frame_offset], bodyer->package[pckidx].frame[frmidx].frame_size);
		}

		message->package.frames_of_package[pckidx] = bodyer->package[pckidx].frames;
		frames += bodyer->package[pckidx].frames;
	}

	message->package.packages = bodyer->packages;
	message->package.frames = frames;

	message->flags = header->compression | header->encryption;
	message->ptype = header->socket_type;
}

int commdata_parse(struct comm_data *commdata)
{
	assert(commdata);

	int idx = 0;

	while (commdata->recv_cache.size > 0) {
		int size = mfptp_parse(&commdata->parser);	/* 成功解析数据的字节数 */

		if (unlikely(size == 0)) {
			/* 数据未接收完毕 */
			return 0;
		}

		if (unlikely(commdata->parser.ms.error != MFPTP_OK)) {
			/* 解析出错 抛弃已解析的错误数据 */
			loger("parse failed data:%*.s\n", commdata->recv_cache.size, &commdata->recv_cache.buffer[commdata->recv_cache.start]);
			return -1;
		}

		/* 成功解析了一个完整的包 */
		if (unlikely(commdata->parser.header.socket_type == HEARTBEAT_METHOD)) {
			/* 心跳包直接丢弃 */
			commdata->recv_cache.start += size;
			commdata->recv_cache.size -= size;
			commcache_adjust(&commdata->recv_cache);
			continue;
		}

		/* 正常的数据包 */
		struct comm_message *message = commmsg_make(NULL, commdata->parser.bodyer.dsize);
		_fill_message_package(message, &commdata->parser);

		commdata->recv_cache.start += size;
		commdata->recv_cache.size -= size;
		commcache_adjust(&commdata->recv_cache);

		if ((message->flags > 0) && !decrypt_decompress_data(message)) {
			/* 解密压缩失败 则直接丢弃此包信息 */
			commmsg_free(message);
			continue;
		}

		commlock_lock(&commdata->recvlock);

		if (unlikely(!commqueue_push(&commdata->recvqueue, (void *)&message))) {
			/* 队列已满，则放入链表中 */
			commlist_push(&commdata->recvlist, &message, sizeof(&message));
		}

		commlock_unlock(&commdata->recvlock);

		// commlock_wake(&commdata->recvlock, false);

		loger("parse successed\n");
		idx++;
	}

	return idx;
}

bool commconnfd_send(struct connfd_info *connfd)
{
	assert(connfd);

	struct comm_data *commdata = &connfd->commdata;

	if (commdata->send_cache.size > 0) {
		int bytes = rsocket_send(&connfd->commtcp.rsocket, &commdata->send_cache.buffer[commdata->send_cache.start], commdata->send_cache.size);

		if (bytes < 0) {
			return false;
		}

		if (bytes > 0) {
			commdata->send_cache.start += bytes;
			commdata->send_cache.size -= bytes;
			commcache_adjust(&commdata->send_cache);
		}

		/* 被打断，则下次继续处理 */
	}

	return true;
}

bool commconnfd_recv(struct connfd_info *connfd)
{
	assert(connfd);

	int     bytes = 0;
	char    buff[COMM_READ_MIOU] = {};

	struct comm_data *commdata = &connfd->commdata;

	while ((bytes = rsocket_recv(&connfd->commtcp.rsocket, buff, COMM_READ_MIOU)) > 0) {
		commcache_append(&commdata->recv_cache, buff, bytes);

		if (bytes < COMM_READ_MIOU) {
			/* 数据已经读取完毕 */
			break;
		}
	}

	/* 判断socket状态 */
	return (bytes < 0) ? false : true;
}

struct comm_evts *commevts_make(struct comm_evts *commevts)
{
	if (commevts) {
		bzero(commevts, sizeof(*commevts));
		commevts->canfree = false;
	} else {
		New(commevts);
		assert(commevts);
		commevts->canfree = true;
	}

	commevts->connfdcnt = 0;
	commevts->bindfdcnt = 0;

	commpipe_create(&commevts->cmdspipe);
	/*init pipe evts*/
	commpipe_create(&commevts->sendpipe);
	// commevts->sendevfd = eventfd(0, EFD_NONBLOCK);
	commpipe_create(&commevts->recvpipe);
	// commevts->recvevfd = eventfd(0, EFD_NONBLOCK);

	commepoll_init(&commevts->commepoll, EPOLL_SIZE);
	commepoll_add(&commevts->commepoll, commevts->cmdspipe.rfd, EPOLLET | EPOLLIN, EVT_TYPE_PIPE);
	commepoll_add(&commevts->commepoll, commevts->sendpipe.rfd, EPOLLET | EPOLLIN, EVT_TYPE_PIPE);

	commevts->init = true;

	return commevts;
}

void commevts_free(struct comm_evts *commevts)
{
	if (commevts && commevts->init) {
		int fd = 0;

		/* 删除所有类型为COMM_CONNECT和COMM_ACCEPT的fd相关信息 */
		while (commevts->connfdcnt) {
			struct connfd_info *connfd = commevts->connfd[fd];

			if (connfd) {
				commepoll_del(&commevts->commepoll, fd, -1, EVT_TYPE_NULL);
				commdata_away(&connfd->commdata);
				close(fd);
				free(connfd);
				loger("close connfd:%d\n", fd);
				commevts->connfdcnt--;
			}

			fd++;
		}

		/* 删除所有类型为COMM_BIND的fd相关信息 */
		while (commevts->bindfdcnt) {
			struct bindfd_info *bindfd = &commevts->bindfd[commevts->bindfdcnt - 1];
			fd = bindfd->commtcp.rsocket.sktfd;
			commepoll_del(&commevts->commepoll, fd, -1, EVT_TYPE_NULL);
			close(fd);
			loger("close bindfd %d\n", fd);
			commevts->bindfdcnt--;
		}

		/* 删除pipe的fd的监控 */
		commepoll_del(&commevts->commepoll, commevts->cmdspipe.rfd, -1, EVT_TYPE_NULL);
		commepoll_del(&commevts->commepoll, commevts->sendpipe.rfd, -1, EVT_TYPE_NULL);

		commpipe_destroy(&commevts->cmdspipe);
		commpipe_destroy(&commevts->sendpipe);
		commpipe_destroy(&commevts->recvpipe);
		// close(commevts->sendevfd);
		// close(commevts->recvevfd);

		commepoll_destroy(&commevts->commepoll);

		commevts->init = false;

		if (commevts->canfree) {
			Free(commevts);
		}
	}
}

bool commevts_socket(struct comm_evts *commevts, struct comm_tcp *commtcp, struct comm_cbinfo *cbinfo)
{
	assert(commevts && commevts->init && commtcp && commtcp->rsocket.sktfd > 0);

	/* 添加一个fd进行监听 */
	if ((commtcp->type == COMM_CONNECT) || (commtcp->type == COMM_ACCEPT)) {
		struct connfd_info *connfd = calloc(1, sizeof(struct connfd_info));
		memcpy(&connfd->commtcp, commtcp, sizeof(*commtcp));

		if (cbinfo) {
			memcpy(&connfd->cbinfo, cbinfo, sizeof(*cbinfo));
		}

		connfd->workstep = STEP_INIT;

		bool ok = commdata_init(&connfd->commdata);

		if (!ok) {
			free(connfd);
			return false;
		}

		commevts->connfd[commtcp->rsocket.sktfd] = connfd;

		commevts->connfdcnt++;
	} else {
		struct bindfd_info *bindfd = &commevts->bindfd[commevts->bindfdcnt];
		memcpy(&bindfd->commtcp, commtcp, sizeof(*commtcp));

		if (cbinfo) {
			memcpy(&bindfd->cbinfo, cbinfo, sizeof(*cbinfo));
		}

		bindfd->workstep = STEP_INIT;

		commevts->bindfdcnt++;
	}

	write(commevts->cmdspipe.wfd, (void *)&commtcp->rsocket.sktfd, sizeof(commtcp->rsocket.sktfd));

	loger("commtcp local port:%s addr:%s peer port:%s addr:%s\n",
		commtcp->localport, commtcp->localaddr, commtcp->peerport, commtcp->peeraddr);
	return true;
}

bool commevts_push(struct comm_evts *commevts, struct comm_message *message)
{
	struct comm_message *commmsg = commmsg_make(NULL, message->package.raw_data.len);

	commmsg_copy(commmsg, message);

	struct connfd_info *connfd = commevts->connfd[message->fd];

	if (connfd) {
		struct comm_data *commdata = &connfd->commdata;
		commlock_lock(&commdata->sendlock);

		if (unlikely(!commqueue_push(&commdata->sendqueue, (void *)&commmsg))) {
			/* 队列已满，则存放到链表中 */
			commlist_push(&commdata->sendlist, &commmsg, sizeof(&commmsg));
		}

		commlock_unlock(&commdata->sendlock);
		/* 发送给对发以触发写事件 如果数据写满则一直堵塞到对方读取数据 */
		write(commevts->sendpipe.wfd, (void *)&message->fd, sizeof(message->fd));
		return true;
	}

	return false;
}

bool commevts_pull(struct comm_evts *commevts, struct comm_message *message)
{
	int     fd = 0;
	int     bytes = 0;

	do {
		// TODO:add epoll
		bytes = read(commevts->recvpipe.rfd, &fd, sizeof(fd));
	} while (bytes != sizeof(fd));

	struct comm_message     *commmsg = NULL;
	struct connfd_info      *connfd = commevts->connfd[fd];

	if (connfd) {
		struct comm_data *commdata = &connfd->commdata;
		commlock_lock(&commdata->recvlock);

		if (unlikely(!commqueue_pull(&commdata->recvqueue, (void *)&commmsg))) {
			/* 队列已空，则存放在链表中 */
			commlist_pull(&commdata->recvlist, &commmsg, sizeof(&commmsg));
		}

		loger("queue nodes:%ld list nodes:%ld\n", commdata->recvqueue.nodes, commdata->recvlist.nodes);

		commlock_unlock(&commdata->recvlock);

		if (commmsg) {
			/* 取到数据 */
			commmsg->fd = fd;
			commmsg_copy(message, commmsg);
			commmsg_free(commmsg);
			return true;
		}
	}

	return false;
}

/* 检测fd是否为COMM_BIND类型的fd并保存于struct listenfd结构体中的数组中  @返回值：-1代表不是，否则返回fd所在数组的下标 */
static inline int gain_bindfd_fdidx(struct comm_evts *commevts, int fd)
{
	assert(commevts && commevts->init && fd > 0);
	int fdidx = 0;

	for (fdidx = 0; fdidx < commevts->bindfdcnt; fdidx++) {
		if (fd == commevts->bindfd[fdidx].commtcp.rsocket.sktfd) {
			return fdidx;
		}
	}

	return -1;
}

void commevts_close(struct comm_evts *commevts, int fd)
{
	assert(commevts && commevts->init && fd > 0);

	struct connfd_info *connfd = commevts->connfd[fd];

	if (connfd) {
		connfd->workstep = STEP_STOP;
		write(commevts->cmdspipe.wfd, (void *)&fd, sizeof(fd));
	} else {
		int fdidx = gain_bindfd_fdidx(commevts, fd);

		if (fdidx > -1) {
			struct bindfd_info *bindfd = &commevts->bindfd[fdidx];
			bindfd->workstep = STEP_STOP;
			write(commevts->cmdspipe.wfd, (void *)&fd, sizeof(fd));
		}
	}
}

/* 接收新用户的连接 */
static int commevts_accept(struct comm_evts *commevts, struct bindfd_info *bindfd)
{
	assert(commevts && bindfd);

	do {
		/* 循环处理accept直到所有新连接都处理完毕退出 */
		int             fd = rsocket_accept(&bindfd->commtcp.rsocket);

		if (fd > 0) {
			struct comm_tcp commtcp = {};
			commtcp.rsocket.sktfd = fd;
			commtcp.type = COMM_ACCEPT;

			assert(commtcp_get_portinfo(&commtcp, true, commtcp.localaddr, commtcp.localport));
			assert(commtcp_get_portinfo(&commtcp, false, commtcp.peeraddr, commtcp.peerport));

			bool ok = commevts_socket(commevts, &commtcp, &bindfd->cbinfo);

			if (!ok) {
				/* fd添加到struct comm_evts中进行监控失败，则忽略并关闭此描述符 */
				close(fd);
				loger("add socket fd to monitor failed\n");
			} else {
				return fd;
			}
		} else if (fd == -1) {
			if (unlikely((errno == ECONNABORTED) || (errno == EPROTO) || (errno == EINTR))) {
				/* 可能被打断，继续处理下一个连接 */
				continue;
			} else {
				/* 处理完毕或者出现致命错误，则直接返回 */
				return -1;
			}
		}
	} while (1);
}

static void do_work_step(struct comm_evts *commevts, int fd)
{
	struct connfd_info *connfd = commevts->connfd[fd];

	if (connfd) {
		/*connect/accept事件.*/
		if (connfd->cbinfo.fcb) {
			connfd->cbinfo.fcb(commevts->commctx, connfd->commtcp.rsocket.sktfd, connfd->workstep, connfd->cbinfo.usr);
		}
		if (connfd->workstep == STEP_INIT) {
			/*open*/
			bool ok = commepoll_add(&commevts->commepoll, fd, EPOLLIN | EPOLLET, EVT_TYPE_SOCK);

			if (!ok) {
				loger("add connect evt faild!\n");
			} else {
				connfd->workstep = STEP_HAND;
			}
		}
		
		if (connfd->workstep == STEP_ERRO) {
			commepoll_del(&commevts->commepoll, fd, -1, EVT_TYPE_NULL);
			//TODO
		}

		if (connfd->workstep == STEP_STOP) {
			/*close*/
			bool ok = commepoll_del(&commevts->commepoll, fd, -1, EVT_TYPE_NULL);

			if (!ok) {
				loger("del connect evt faild!\n");
			}

			commdata_away(&connfd->commdata);
			free(connfd);
			commevts->connfd[fd] = NULL;
			commevts->connfdcnt--;
			rsocket_close(&connfd->commtcp.rsocket);
		}
	} else {
		int fdidx = gain_bindfd_fdidx(commevts, fd);

		if (fdidx >= 0) {
			/*listen事件.*/
			struct bindfd_info *bindfd = &commevts->bindfd[fdidx];

			if (bindfd->cbinfo.fcb) {
				bindfd->cbinfo.fcb(commevts->commctx, bindfd->commtcp.rsocket.sktfd, bindfd->workstep, bindfd->cbinfo.usr);
			}
			if (bindfd->workstep == STEP_INIT) {
				/*open*/
				bool ok = commepoll_add(&commevts->commepoll, fd, EPOLLIN | EPOLLET, EVT_TYPE_SOCK);

				if (!ok) {
					loger("add listen evt faild!\n");
				} else {
					bindfd->workstep = STEP_HAND;
				}
			}

			if (bindfd->workstep == STEP_ERRO) {
				commepoll_del(&commevts->commepoll, fd, -1, EVT_TYPE_NULL);
				//TODO
			}

			if (bindfd->workstep == STEP_STOP) {
				/*close*/
				bool ok = commepoll_del(&commevts->commepoll, fd, -1, EVT_TYPE_NULL);

				if (!ok) {
					loger("del listen evt faild!\n");
				}

				if ((fdidx + 1) != commevts->bindfdcnt) {
					/* 如果删除的fd不是最后一个fd，则将后面的fd数据往前拷贝 */
					int size = sizeof(struct bindfd_info) * (commevts->bindfdcnt - fdidx - 1);
					memmove(&commevts->bindfd[fdidx], &commevts->bindfd[fdidx + 1], size);
				}

				commevts->bindfdcnt--;
				rsocket_close(&bindfd->commtcp.rsocket);
			}
		}
	}
}

#define PIPE_READ_MIOU 64	/* 一次性读取数据的大小 */
void commevts_once(struct comm_evts *commevts)
{
	bool have = commepoll_wait(&commevts->commepoll, EPOLLTIMEOUTED);

	if (!have) {
		return;
	}

	/*有事件*/
	loger("commepoll_wait have events\n");
	int n = 0;

	for (n = 0; n < commevts->commepoll.eventcnt; n++) {
		struct epoll_event      *evts = &commevts->commepoll.events[n];
		int                     fhand = commepoll_get_event_fhand(evts);
		char                    ftype = commepoll_get_event_ftype(evts);
		assert(fhand > 0);

		if (ftype == EVT_TYPE_PIPE) {
			/* 有pipe事件触发 */
			if (!(evts->events & EPOLLIN)) {
				continue;
			}

			/* 管道可读事件 */
			loger("pipe event start\n");

			int     fda[PIPE_READ_MIOU] = { 0 };
			int     bytes = read(fhand, fda, sizeof(fda));
			int     cnt = bytes / sizeof(int);

			if (cnt == 0) {
				continue;
			}

			int i = 0;

			for (i = 0; i < cnt; i++) {
				int fd = fda[i];

				if (fhand == commevts->cmdspipe.rfd) {
					struct connfd_info *connfd = commevts->connfd[fd];
					if (connfd && (connfd->commtcp.type == COMM_CONNECT)
							&& (connfd->workstep == STEP_INIT)) {
						if (unlikely(rsocket_connect(&connfd->commtcp.rsocket))) {
							loger("connect socket failed\n");
							connfd->workstep = STEP_WAIT;
						} else {
							connfd->workstep = STEP_INIT;
						}
						assert(commtcp_get_portinfo(&connfd->commtcp, true, connfd->commtcp.localaddr, connfd->commtcp.localport));
					}
#if 0
					//TODO: add FD_ERRO to commapi_close()
#endif

					do_work_step(commevts, fd);
				}

				if (fhand == commevts->sendpipe.rfd) {
					struct connfd_info *connfd = commevts->connfd[fd];

					if (connfd && (connfd->workstep == STEP_HAND)) {
						/*打包发送事件*/
						bool ok = commconnfd_send(connfd);

						if (!ok) {
							connfd->workstep = STEP_ERRO;
							do_work_step(commevts, fd);
							continue;
						}

						/* 处理打包事件 */
						int ret = commdata_package(&connfd->commdata);

						if (ret > 0) {
							ok = commconnfd_send(connfd);

							if (!ok) {
								connfd->workstep = STEP_ERRO;
								do_work_step(commevts, fd);
								continue;
							}
						}

						if (connfd->commdata.send_cache.size) {
							commepoll_mod(&commevts->commepoll, fd, EPOLLIN | EPOLLET | EPOLLOUT, EVT_TYPE_SOCK);
						}
					}
				}
			}
		} else {
			/* 有sock事件触发 */
			struct connfd_info *connfd = commevts->connfd[fhand];

			if (connfd) {
				if (evts->events & EPOLLERR) {
					connfd->workstep = STEP_ERRO;
					do_work_step(commevts, fhand);
					continue;
				}

				if (evts->events & EPOLLIN) {
					/* socket的fd触发了读事件 */
					bool ok = commconnfd_recv(connfd);

					if (!ok) {
						connfd->workstep = STEP_ERRO;
						do_work_step(commevts, fhand);
						continue;
					}

					/* 处理解析事件 */
					int ret = commdata_parse(&connfd->commdata);

					if (ret == -1) {
						// TODO:设置断开连接标志
						connfd->workstep = STEP_ERRO;
						do_work_step(commevts, fhand);
						continue;
					}

					/*写入管道*/
					int i = 0;

					for (i = 0; i < ret; i++) {
						write(commevts->recvpipe.wfd, (void *)&fhand, sizeof(fhand));
					}
				}

				if (evts->events & EPOLLOUT) {
					/* socket的fd触发了写事件 */
					bool ok = commconnfd_send(connfd);

					if (!ok) {
						connfd->workstep = STEP_ERRO;
						do_work_step(commevts, fhand);
						continue;
					}

					if (!connfd->commdata.send_cache.size) {
						commepoll_mod(&commevts->commepoll, fhand, EPOLLIN | EPOLLET, EVT_TYPE_SOCK);
					}
				}
			} else {
				int fdidx = gain_bindfd_fdidx(commevts, fhand);

				if (fdidx >= 0) {
					/* 新的客户端连接,触发accept事件 */
					struct bindfd_info      *bindfd = &commevts->bindfd[fdidx];
					int                     fd = commevts_accept(commevts, bindfd);

					struct connfd_info *connfd = commevts->connfd[fd];

					connfd->workstep = STEP_INIT;
					do_work_step(commevts, fd);

					loger("listen fd:%d accept fd:%d\n", bindfd->commtcp.rsocket.sktfd, fd);
					loger("accept fd localport: %s local addr:%s peerport:%s peeraddr:%s\n",
						connfd->commtcp.localport, connfd->commtcp.localaddr,
						connfd->commtcp.peerport, connfd->commtcp.peeraddr);
				}
			}
		}
	}
}

