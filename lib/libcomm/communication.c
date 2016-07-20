/*********************************************************************************************/
/************************	Created by 许莉 on 16/02/25.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include <dirent.h>
#include <stdlib.h>
#include "communication.h"

#define QUEUE_NODES     1024		/* 队列里面可以存放多少个节点的数据 */
#define EPOLLTIMEOUTED  5000		/* epoll_wait的超时事件 以毫秒(ms)为单位 1s = 1000ms*/
#define	PIPEVALUE	1000*10		/* 从开始管道读取数据计时器开始到下一次读取管道的时间长度 [单位:ms] */
#define PIPEINTERVAL	1000*20		/* 没间隔5s去读取管道里面的信息 [单位:ms] */

static void *_start_new_pthread(void *usr);

static void  _set_remainfd(struct comm_timer *commtimer, struct comm_list *timerhead, void *usr);

static bool _check_packageinfo(const struct comm_message *message);

struct comm_context *comm_ctx_create(int epollsize)
{
	struct comm_context *commctx = NULL;

	New(commctx);

	if (unlikely(!commctx)) {
		goto error;
	}

	if (unlikely(!commevent_init(&commctx->commevent, commctx))) {
		goto error;
	}

	if (unlikely(!commqueue_init(&commctx->recvqueue, sizeof(intptr_t), QUEUE_NODES, free_commmsg))) {
		goto error;
	}

	if (unlikely(!commlock_init(&commctx->statlock))) {
		goto error;
	}

	if (unlikely(!commlock_init(&commctx->recvlock))) {
		goto error;
	}

	if (unlikely(!commpipe_create(&commctx->commpipe))) {
		goto error;
	}

	if (unlikely(!commepoll_init(&commctx->commepoll, ((epollsize > 0) && (epollsize <= EPOLL_SIZE)) ? epollsize : EPOLL_SIZE))) {
		goto error;
	}

	if (unlikely(!commepoll_add(&commctx->commepoll, commctx->commpipe.rfd, EPOLLET | EPOLLIN))) {
		goto error;
	}

	if (unlikely((pthread_create(&commctx->ptid, NULL, _start_new_pthread, (void *)commctx))) != 0) {
		goto error;
	}

	commctx->pipetimer = commtimer_create(PIPEVALUE, PIPEINTERVAL, _set_remainfd, (void*)commctx);
	if (commctx->pipetimer == NULL) {
		goto error;
	}

	commlist_init(&commctx->recvlist, free_commmsg);
	commlist_init(&commctx->timerhead, NULL);
	commcache_init(&commctx->cache);
	commctx->stat = COMM_STAT_INIT;

	return commctx;

error:

	if (commctx) {
		commqueue_destroy(&commctx->recvqueue);
		commlock_destroy(&commctx->statlock);
		commlock_destroy(&commctx->recvlock);
		commpipe_destroy(&commctx->commpipe);

		if (commctx->commepoll.init && (commctx->commepoll.watchcnt > 0)) {
			/* 如果pipe的fd被监听成功则删除pipe的fd监控 */
			commepoll_del(&commctx->commepoll, commctx->commpipe.rfd, -1);
		}

		commevent_destroy(commctx->commevent);
		commepoll_destroy(&commctx->commepoll);
		commlist_destroy(&commctx->recvlist, COMMMSG_OFFSET);
		commcache_free(&commctx->cache);
		Free(commctx);
	}

	return NULL;
}

void comm_ctx_destroy(struct comm_context *commctx)
{
	if (commctx) {
		/* 在子线程还没被唤醒的时候就调用销毁函数则需要先唤醒子线程 */
		if (unlikely(commctx->stat == COMM_STAT_INIT)) {
			commlock_wake(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_RUN, false);
		}

		ATOMIC_SET(&commctx->stat, COMM_STAT_STOP);

		/* 等待子线程退出然后再继续销毁数据 */
		commlock_wait(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_NONE, -1, false);
		pthread_join(commctx->ptid, NULL);

		commevent_destroy(commctx->commevent);
		commqueue_destroy(&commctx->recvqueue);
		commlock_destroy(&commctx->statlock);
		commlock_destroy(&commctx->recvlock);

		/* 删除pipe的fd的监控 */
		commepoll_del(&commctx->commepoll, commctx->commpipe.rfd, -1);
		commpipe_destroy(&commctx->commpipe);
		commlist_destroy(&commctx->recvlist, COMMMSG_OFFSET);
		commepoll_destroy(&commctx->commepoll);
		commtimer_destroy(commctx->pipetimer);
		Free(commctx);
	}
}

int comm_socket(struct comm_context *commctx, const char *host, const char *service, struct cbinfo *finishedcb, int flag)
{
	assert(commctx && host && service);

	struct comm_tcp commtcp = {};

	if ((flag & 0x0F) == COMM_BIND) {
		if (commctx->commevent->bindfdcnt < LISTEN_SIZE) {
			if (unlikely(!socket_listen(&commtcp, host, service))) {
				log("bind socket failed\n");
				return -1;
			}
		} else {
			log("bind too many socket in one comm_context\n");
			return -1;
		}
	} else {
		if (unlikely(!socket_connect(&commtcp, host, service, (flag & 0xF0)))) {
			log("connect socket failed\n");
			return -1;
		}
	}

	/* 添加一个fd进行监听 */
	if (unlikely(!commdata_add(commctx->commevent, &commtcp, finishedcb))) {
		log("add socket fd to monitor failed\n");
		close(commtcp.fd);
		return -1;
	}

	//log("commtcp local port:%d addr:%s\n", commtcp.localport, commtcp.localaddr);
	//log("commtcp peer port:%d addr:%s\n", commtcp.peerport, commtcp.peeraddr);

	/* 将状态值设置为COMM_STAT_RUN并唤醒等待的线程 */
	if (commctx->stat == COMM_STAT_INIT) {
		commlock_wake(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_RUN, false);
	}
	return commtcp.fd;
}

/* 实际发送数据无需堵塞，@block @timeout为保留参数 目前没有使用 */
int comm_send(struct comm_context *commctx, const struct comm_message *message, bool block, int timeout)
{
	assert(commctx && message && message->content);
	assert(message->fd > 0 && message->fd < EPOLL_SIZE);	/* 保证描述符在范围内 */

	struct connfd_info      *connfd = NULL;
	struct comm_message     *commmsg = NULL;

	if ((connfd = commctx->commevent->connfd[message->fd])) {
		if (unlikely(!_check_packageinfo(message))) {
			return -1;
		}
		if (new_commmsg(&commmsg, message->package.dsize)) {
			copy_commmsg(commmsg, message);

			commlock_lock(&connfd->sendlock);

			if (unlikely(!commqueue_push(&connfd->send_queue, (void *)&commmsg))) {
				/* 队列已满，则存放到链表中 */
				commlist_push(&connfd->send_list, &commmsg->node);

			}

			commlock_unlock(&connfd->sendlock);

			/* 发送给对发以触发写事件 如果数据写满则一直堵塞到对方读取数据 */
			commpipe_write(&commctx->commpipe, (void *)&message->fd, sizeof(message->fd));
			return message->package.dsize;
		}
	}

	return -1;
}

int comm_recv(struct comm_context *commctx, struct comm_message *message, bool block, int timeout)
{
	assert(commctx && message && message->content);

	bool                    flag = false;
	struct comm_message     *commmsg = NULL;

	commlock_lock(&commctx->recvlock);
	do {
		if (unlikely(!commqueue_pull(&commctx->recvqueue, (void *)&commmsg))) {
			struct comm_list *list = NULL;

			if (commlist_pull(&commctx->recvlist, (void *)&list)) {
				commmsg = (struct comm_message *)get_container_addr(list, COMMMSG_OFFSET);
				flag = false;
			//	break;
			} else if (block) {
				commctx->recvqueue.readable = 0;

				if (commlock_wait(&commctx->recvlock, &commctx->recvqueue.readable, 1, timeout, true)) {
					flag = true;			/* 返回值为真说明有数据可读 */
				}
			} else {
				commctx->recvqueue.readable = 1;	/* 不进行堵塞就设置为1不需要唤醒 */
			}
		} else {
			flag = false;
		//	break;
		}
		if (commmsg) {
			commmsg->connfd->msgcounter --;
			if (commmsg->connfd->commtcp.stat == FD_CLOSE) {
				if (block) {
					flag = true;	/* 此条消息直接丢弃 */
				} else {
					flag = false;
				}
				log("fd closed comm_recv:%d\n",commmsg->fd);
				if (commmsg->connfd->msgcounter == 0 && commmsg->connfd->commtcp.connattr != CONNECT_ANYWAY) {
					//commdata_del(commctx->commevent, commmsg->fd);
					commdata_destroy(commmsg->connfd);
				}
				free_commmsg(commmsg);
				commmsg = NULL;
			}
			commmsg->connfd = NULL;	/* 此变量只是用来判断此fd是否已经被关闭 */
		}
		log("queue nodes:%d list nodes:%ld\n", commctx->recvqueue.nodes, commctx->recvlist.nodes);
	} while (flag);								/* flag为true说明成功等待到数据 尝试再去取一次数据 */
	commlock_unlock(&commctx->recvlock);

	if (commmsg) {					/* 取到数据 */
		copy_commmsg(message, commmsg);
		//log("\x1B[1;32m""message socket_type:%d\n""\x1B[m", message->socket_type);
		free_commmsg(commmsg);			/* 释放掉comm_message结构体 */
		return message->package.dsize;
	}

	return -1;
}

void comm_settimeout(struct comm_context *commctx, int timeout, CommCB callback, void *usr)
{
	commctx->commevent->timeoutcb.timeout = timeout;
	commctx->commevent->timeoutcb.callback = callback;
	commctx->commevent->timeoutcb.usr = usr;
}

void comm_close(struct comm_context *commctx, int fd)
{
	commdata_del(commctx->commevent, fd);
	close(fd);
}

/* 一个新的线程开始运行 */
static void *_start_new_pthread(void *usr)
{
	struct comm_context *commctx = (struct comm_context *)usr;

	assert(commctx);

	int     n = 0;
	int     fdidx = 0;	/* 监听fd在结构体struct listenfd中数组的下标 */
	bool    retval = false;
	struct remainfd *remainfd = &commctx->commevent->remainfd;

	/* 等待主线程将状态设置为COMM_STAT_RUN，才被唤醒继续执行以下代码 */
	commlock_wait(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_RUN, -1, false);

	/* 启动管道读取信息计时器 */
	log("start pipe timer\n");
	commtimer_start(commctx->pipetimer, &commctx->timerhead);

	while (1) {
		/* 线程状态为STOP的时候则将状态设置为NONE，返回真，则代表设置成功，退出循环 */
		if (unlikely(commctx->stat == COMM_STAT_STOP)) {
			commlock_wake(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_NONE, false);
			break;
		}

		/* 开始epoll_wait等待描述符就绪 */
		if (remainfd->cnt[0] || remainfd->cnt[1] || remainfd->cnt[2] || remainfd->cnt[3] || remainfd->cnt[4]) {
			/* 有任务需要处理 则不进行堵塞等待事件的触发 */
			commctx->commevent->timeoutcb.timeout = 0;
		} else {
			/* 用户没有设置epoll_wait超时，则使用默认超时时间 */
			if (commctx->commevent->timeoutcb.timeout < 1) {
				commctx->commevent->timeoutcb.timeout = EPOLLTIMEOUTED;
			}
		};

		//log("commepoll_wait start\n");
		if ((retval = commepoll_wait(&commctx->commepoll, commctx->commevent->timeoutcb.timeout))) {
			for (n = 0; n < commctx->commepoll.eventcnt; n++) {
				if (commctx->commepoll.events[n].data.fd > 0) {
					/* 新的客户端连接， 触发accept事件 */
					if ((fdidx = gain_bindfd_fdidx(commctx->commevent, commctx->commepoll.events[n].data.fd)) > -1) {
						add_remainfd(remainfd, fdidx, REMAINFD_LISTEN);
					} else if (commctx->commepoll.events[n].events & EPOLLIN) {			/* 有数据可读，触发读数据事件 */
						if (commctx->commepoll.events[n].data.fd == commctx->commpipe.rfd) {	/* 管道事件被触发，则触发打包事件 */
							//log("pipe event start\n");
							_set_remainfd(NULL, NULL, (void*)commctx);
						} else {	/* 非pipe的fd读事件被触发，则代表是socket的fd触发了读事件 */
							add_remainfd(remainfd, commctx->commepoll.events[n].data.fd, REMAINFD_READ);
						}
					} else if (commctx->commepoll.events[n].events & EPOLLOUT) {			/* 有数据可写， 触发写数据事件 */
						struct connfd_info* connfd = (struct connfd_info*)commctx->commevent->connfd[commctx->commepoll.events[n].data.fd];
						if (connfd && connfd->commtcp.stat != FD_INIT) {	/* 不属于添加到epoll里面监控写事件时强制触发 */
							add_remainfd(remainfd, commctx->commepoll.events[n].data.fd, REMAINFD_WRITE);
						}
					}
				}
			}
		}
		//log("commepoll_wait over\n");

		if ((retval == false) && (errno == EINTR)) {
			/* epoll_wait超时调用处理残留fd函数 @true会调用用户的超时回调函数 */
			commevent_remainfd(commctx->commevent, true);
		} else {
			/* epoll_wait每次循环完一次就去处理一次残留的fd */
			commevent_remainfd(commctx->commevent, false);
		}
		commtimer_scheduler(&commctx->timerhead);
	}
	/* 停止管道读取信息计时器 */
	commtimer_stop(commctx->pipetimer, &commctx->timerhead);
	log("stop pipe timer\n");

	return NULL;
}

static void  _set_remainfd(struct comm_timer *commtimer, struct comm_list *timerhead, void *usr)
{
	assert(usr);
	int cnt = 0, i = 0;
	int fda[EPOLL_SIZE] = {};
	struct comm_context *commctx = (struct comm_context*)usr;

//	log("deal with pipe timer\n");
	if ((cnt = commpipe_read(&commctx->commpipe, fda, sizeof(int))) > 0) {
		for (i = 0; i < cnt; i++) {
			add_remainfd(&commctx->commevent->remainfd, fda[i], REMAINFD_PACKAGE);
	//		log("read pipe fd:%d\n", fda[i]);
		}
	}
}

/* 检测包的信息是否设置正确 */
static bool _check_packageinfo(const struct comm_message *message)
{
	assert(message);
	int     index = 0;
	int     dsize = 0;	/* 数据的总大小 */
	int     pckidx = 0;	/* 包的索引 */
	int     frmidx = 0;	/* 帧的索引 */
	int	frames = 0;	/* 帧的总数 */

	if (unlikely(message->package.packages < 1)) {
		log("wrong packages in comm_message structure, packages:%d", message->package.packages);
		return false;
	}
	for (pckidx = 0; pckidx < message->package.packages; pckidx++) {
		if (message->package.frames_of_package[pckidx] < 1 || message->package.frames_of_package[pckidx] > message->package.frames) { 
			log("wrong sum of frames in frames_of_pack of comm_message structure, frames:%d, index:%d\n", message->package.frames_of_package[pckidx], pckidx);
			return false;
		}
		for (frmidx = 0 ; frmidx < message->package.frames_of_package[pckidx]; frmidx++, index++) {
			if (unlikely(dsize > message->package.dsize || message->package.frame_size[index] > message->package.dsize)) {
				log("wrong frame_size in comm_message structure, frame_size:%d index:%d\n", message->package.frame_size[index], index);
				return false;
			}
			if (unlikely(message->package.frame_offset[index] != dsize)) {
				log("wrong frame_offset in comm_package, frame_offset:%d index:%d\n", message->package.frame_offset[index], index);
				return false;
			}
			dsize += message->package.frame_size[index];
			frames++;
		}
	}

	if (unlikely(frames != message->package.frames)) {
		log("wrong sum of frames in comm_message structure, frames:%d\n",message->package.frames);
		return false;
	}
	if (unlikely(dsize != message->package.dsize)) {
		log("wrong sum of datasize in comm_message structure, datasize:%d\n", message->package.dsize);
		return false;
	}
	return true;
}
