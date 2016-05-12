/*********************************************************************************************/
/************************	Created by 许莉 on 16/02/25.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include <dirent.h>
#include <stdlib.h>

#include "loger.h"
#include "communication.h"


static void * _start_new_pthread(void* usr);


struct comm_context* comm_ctx_create(int epollsize)
{
	struct comm_context*	commctx = NULL;

	New(commctx);
	if (unlikely(!commctx)) {
		goto error;
	}

	commctx->listenfd = -1;
	if (unlikely( !commqueue_init(&commctx->recv_queue, QUEUE_CAPACITY, sizeof(intptr_t)))) {
		goto error;
	}

	if (unlikely(!commlock_init(&commctx->statlock))) {
		goto error;
	}

	if (unlikely(!commlock_init(&commctx->recvlock))) {
		goto error;
	}

	if (unlikely(!commpipe_init(&commctx->commpipe))) {
		goto error;
	}

	if (unlikely(!commepoll_init(&commctx->commepoll, EPOLL_SIZE))) {
		goto error;
	}

	if (unlikely(!commepoll_add(&commctx->commepoll, commctx->commpipe.rfd, EPOLLET | EPOLLIN))) {
		goto error;
	}

	if (unlikely((pthread_create(&commctx->ptid, NULL, _start_new_pthread, (void*)commctx))) != 0) {
		goto error;
	}
	commctx->stat = COMM_STAT_INIT; 

	return commctx;
error:
	if (commctx) {
		commqueue_destroy(&commctx->recv_queue);
		commlock_destroy(&commctx->statlock);
		commlock_destroy(&commctx->recvlock);
		commpipe_destroy(&commctx->commpipe);
		if (commctx->commepoll.init && commctx->commepoll.watchcnt > 0) {
			commepoll_del(&commctx->commepoll, commctx->commpipe.rfd, -1);
		}
		commepoll_destroy(&commctx->commepoll);
		Free(commctx);
	}
	return NULL;
}

void comm_ctx_destroy(struct comm_context* commctx)
{
	int fd = 0;
	if (likely(commctx)) {
		log("destroy everything\n");
		ATOMIC_SET(&commctx->stat, COMM_STAT_STOP);

		/* 等待子线程退出然后再继续销毁数据 */
		log("wait here\n");
		commlock_wait(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_NONE, -1, false);
		log("wait over\n");

		while (commctx->commepoll.watchcnt-1) {	/* 除去commpipe的一个监控fd*/
			if(likely(commctx->data[fd ])) {
				comm_close(commctx, fd);
				log("close fd:%d in comm_ctx_destroy\n", fd);
			}
			fd ++;
			log("here: %d\n", fd);
		}

		commqueue_destroy(&commctx->recv_queue);
		commlock_destroy(&commctx->statlock);
		commlock_destroy(&commctx->recvlock);
		commpipe_destroy(&commctx->commpipe);

		if (commctx->commepoll.init && commctx->commepoll.watchcnt > 0) {
			commepoll_del(&commctx->commepoll, commctx->commpipe.rfd, -1);
		}
		commepoll_destroy(&commctx->commepoll);
		pthread_join(commctx->ptid, NULL);
		Free(commctx);
	}
	return ;
}

int comm_socket(struct comm_context *commctx, const char *host, const char *service, struct cbinfo* finishedcb, int type)
{
	assert(commctx && host && service);

	int			fd = -1;
	bool			retval = false;
	struct portinfo		portinfo = {};
	struct comm_data*	commdata = NULL;

	if (type == COMM_BIND) {
		fd = socket_listen(host, service);
		commctx->listenfd = fd;
	} else {
		fd = socket_connect(host, service);
	}

	if (unlikely(fd < 0)) {
		return fd;
	}

	if (unlikely(!get_portinfo(&portinfo, fd, type, FD_INIT))) {
		close(fd);
		return -1;
	}
	commdata = commdata_init(commctx, &portinfo, finishedcb);
	if (unlikely(!commdata)) {
		close(fd);
		return -1;
	}
	commctx->data[fd ] = (intptr_t)commdata;

	/* 将状态值设置为COMM_STAT_RUN并唤醒等待的线程 */
	commlock_wake(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_RUN, false);
	
	return fd;
}

int comm_send(struct comm_context *commctx, const struct comm_message *message, bool block, int timeout)
{
	assert(commctx && message && message->content);
	assert(message->fd > 0 && message->fd < EPOLL_SIZE); /* 保证描述符在范围内 */

	bool			flag = false;
	struct comm_message	*commmsg = NULL;
	struct comm_data	*commdata = (struct comm_data *)commctx->data[message->fd];
	if (likely(commdata)) {
		commmsg = new_commmsg(message->package.dsize);
		if (likely(commmsg)) {
			copy_commmsg(commmsg, message);
			commlock_lock(&commdata->sendlock);
			while (1) {
				if (likely(commqueue_push(&commdata->send_queue, (void*)&commmsg))) {
					if (!commdata->send_queue.readable) {	/* 此变量为0说明有线程等待可读 */
						commlock_wake(&commdata->sendlock, &commdata->send_queue.readable, 1, true);
					}
					flag = true;
					break ;
				} else if (block) {
					commdata->send_queue.writeable = 0;	/* 将此变量设置为0, 让其他线程进行唤醒 */
					/* 目前打包取数据只在写事件里面被调用，所以必须唤醒写事件才能等待才能被唤醒 */
					commpipe_write(&commctx->commpipe, (void*)&message->fd, sizeof(message->fd));
					if (likely(commlock_wait(&commdata->sendlock, &commdata->send_queue.writeable, 1, timeout, true))) {
						continue ;			/* 返回值为真则说明有空间可写数据 */
					} else {		
						break ;				/* 返回值为假则超时唤醒没有空间可写数据 */
					}
				} else {
					commdata->send_queue.writeable = 1;	/* 不进行堵塞就设置为1不需要唤醒 */
					break ;
				}
			}

			if (unlikely(block && commdata->send_queue.nodes == commdata->send_queue.capacity)) {
				commdata->send_queue.writeable = 0;
			}

			commlock_unlock(&commdata->sendlock);
			if (likely(flag)) {
				/* 数据写入成功 往此管道写入fd 触发子线程的写事件 */
				commpipe_write(&commctx->commpipe, (void*)&message->fd, sizeof(message->fd));
				return message->package.dsize;
			} else {
				free_commmsg(commmsg);				/* push数据失败则需要释放commmsg */
			}
		}
	}
	return -1;
}

int comm_recv(struct comm_context *commctx, struct comm_message *message, bool block, int timeout)
{
	assert(commctx && message && message->content);

	bool			flag = false;
	struct comm_message	*commmsg = NULL;
	commlock_lock(&commctx->recvlock);
	while (1) {
		if (likely(commqueue_pull(&commctx->recv_queue, (void*)&commmsg))) {
			if (!commctx->recv_queue.writeable) {			/* 此变量为0说明有线程等待可写 */
				commlock_wake(&commctx->recvlock, &commctx->recv_queue.writeable, 1, true);
			}
			flag = true;
			break ;
		} else if (block) {
			commctx->recv_queue.readable = 0;
			if (commlock_wait(&commctx->recvlock, &commctx->recv_queue.readable, 1, timeout, true)) {
				continue ;					/* 返回值为真说明有数据可读 */
			} else {
				break ;						/* 返回值为假说明超时唤醒，无数据可读 */
			}
		} else {
			commctx->recv_queue.readable = 1;			/* 不进行堵塞就设置为1不需要唤醒 */
			break ;
		}
	}
	if (unlikely(block && commctx->recv_queue.nodes == 0)) {		/* 当节点数为0时并设置了block时设置此变量等待别人唤醒 */
		commctx->recv_queue.readable = 0;
	}
	commlock_unlock(&commctx->recvlock);

	if (flag) {
		copy_commmsg(message, commmsg);
		free_commmsg(commmsg);						/* 释放掉comm_message结构体 */
		return message->package.dsize;
	}
	return -1;
}

void comm_settimeout(struct comm_context *commctx, int timeout, CommCB callback, void *usr)
{
	commctx->timeoutcb.timeout = timeout;
	commctx->timeoutcb.callback  = callback;
	commctx->timeoutcb.usr = usr;
}

void comm_close(struct comm_context *commctx, int fd)
{
	if (likely(commctx)) {
		if (likely(commctx->data[fd ])) {
			commdata_destroy((struct comm_data*)commctx->data[fd ]);
			commctx->data[fd ] = 0;
		}
		close(fd);
	}
	return ;
}

/* 一个新的线程开始运行 */
static void * _start_new_pthread(void* usr)
{
	assert(usr);
	int			n = 0;
	struct comm_context*	commctx = (struct comm_context*)usr;

	/* 等待主线程将状态设置为COMM_STAT_RUN，才被唤醒继续执行以下代码 */
	commlock_wait(&commctx->statlock, (int*)&commctx->stat, COMM_STAT_RUN, -1, false);

	while (1) {
		/* 线程状态为STOP的时候则将状态设置为NONE，返回真，则代表设置成功，退出循环 */
		if (unlikely(commctx->stat == COMM_STAT_STOP)) {
			log("I am break out\n");
			commlock_wake(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_NONE, false);
			break;
		}					

		/* 用户没有设置epoll_wait超时，则使用默认超时时间 */
		if (commctx->timeoutcb.timeout <= 0) {
			commctx->timeoutcb.timeout = TIMEOUTED;
		}

		if (likely(commepoll_wait(&commctx->commepoll, commctx->timeoutcb.timeout))) {
			for (n = 0; n < commctx->commepoll.eventcnt; n++) {
				if (unlikely(commctx->commepoll.events[n].data.fd < -1)) {
					continue ;
				}

				if (commctx->listenfd != -1 && commctx->commepoll.events[n].data.fd == commctx->listenfd) {	/* 新用户连接，触发accept事件 */
					 accept_event(commctx);

				} else if (commctx->commepoll.events[n].events & EPOLLIN) {					/* 有数据可读，触发读数据事件 */
				
					if (commctx->commepoll.events[n].data.fd == commctx->commpipe.rfd) {			/* 管道事件被触发， 开始写事件 */
						int array[EPOLL_SIZE] = {0};
						int cnt = commpipe_read(&commctx->commpipe, array, sizeof(int));
						if (likely(cnt > 0)) {
							 send_event(commctx, array, cnt);
						} else {
							continue ;
						}
					} else {
						 recv_event(commctx, commctx->commepoll.events[n].data.fd);
					}
				} else if (commctx->commepoll.events[n].events & EPOLLOUT) {					/* 有数据可写， 触发写数据事件 */
					
					int array[1] = {commctx->commepoll.events[n].data.fd};
					 send_event(commctx, array, 1);
				}
			}
		} else {	
			if (likely(errno == EINTR)) {		/* epoll 超时 */
				if(commctx->timeoutcb.timeout > 0){
					timeout_event(commctx);
				}
			}
		}
	}
	return NULL;
}
