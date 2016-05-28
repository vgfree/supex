/*********************************************************************************************/
/************************	Created by 许莉 on 16/02/25.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include <dirent.h>
#include <stdlib.h>
#include "communication.h"

#define	QUEUE_NODES	1024	/* 队列里面可以存放多少个节点的数据 */
#define TIMEOUTED	5000	/* 以毫秒(ms)为单位 1s = 1000ms*/

static void * _start_new_pthread(void* usr);

static bool _set_remainfd(struct remainfd *remainfd, int array[], int n);

static bool _quick_sort(int array[], int n);

struct comm_context* comm_ctx_create(int epollsize)
{
	struct comm_context*	commctx = NULL;

	New(commctx);
	if (unlikely(!commctx)) {
		goto error;
	}

	commlist_init(&commctx->msghead, free_commmsg);
	commevent_init(&commctx->commevent, commctx);
	if (unlikely( !commqueue_init(&commctx->recvqueue, sizeof(intptr_t), QUEUE_NODES, free_commmsg))) {
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

	if (unlikely(!commepoll_init(&commctx->commepoll, (epollsize > 0 ) ? epollsize : EPOLL_SIZE))) {
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
		commqueue_destroy(&commctx->recvqueue);
		commlock_destroy(&commctx->statlock);
		commlock_destroy(&commctx->recvlock);
		commpipe_destroy(&commctx->commpipe);
		if (commctx->commepoll.init && commctx->commepoll.watchcnt > 0) {
			commepoll_del(&commctx->commepoll, commctx->commpipe.rfd, -1);
		}
		commevent_destroy(commctx->commevent);
		commepoll_destroy(&commctx->commepoll);
		Free(commctx);
	}
	return NULL;
}

void comm_ctx_destroy(struct comm_context* commctx)
{
	int fd = 0;
	if (likely(commctx)) {
		if (commctx->stat == COMM_STAT_INIT) {
			/* 在子线程还没被唤醒的时候就调用销毁函数则需要先唤醒子线程 */
			commlock_wake(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_RUN, false);
		};
		ATOMIC_SET(&commctx->stat, COMM_STAT_STOP);

		/* 等待子线程退出然后再继续销毁数据 */
		commlock_wait(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_NONE, -1, false);

		commqueue_destroy(&commctx->recvqueue);
		commlock_destroy(&commctx->statlock);
		commlock_destroy(&commctx->recvlock);
		commpipe_destroy(&commctx->commpipe);

		if (commctx->commepoll.init && commctx->commepoll.watchcnt > 0) {
			commepoll_del(&commctx->commepoll, commctx->commpipe.rfd, -1);
		}

		commlist_destroy(&commctx->msghead, COMMMSG_OFFSET);
		commevent_destroy(commctx->commevent);
		commepoll_destroy(&commctx->commepoll);
		pthread_join(commctx->ptid, NULL);
		Free(commctx);
	}
	return ;
}


int comm_socket(struct comm_context *commctx, const char *host, const char *service, struct cbinfo* finishedcb, int type)
{
	assert(commctx && host && service);

	bool			retval = false;
	struct comm_tcp		commtcp = {};
	struct comm_data*	commdata = NULL;

	if (type == COMM_BIND) {
		if (unlikely(!socket_listen(&commtcp, host, service))) {
			return -1;
		}
	} else {
		if (unlikely(!socket_connect(&commtcp, host, service))) {
			return -1;
		}
	}

	if (unlikely(!commevent_add(commctx->commevent, &commtcp, finishedcb))) {
		close(commtcp.fd);
		return -1;
	}

	/* 将状态值设置为COMM_STAT_RUN并唤醒等待的线程 */
	commlock_wake(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_RUN, false);
	
	return commtcp.fd;
}

int comm_send(struct comm_context *commctx, const struct comm_message *message, bool block, int timeout)
{
	assert(commctx && message && message->content);
	assert(message->fd > 0 && message->fd < EPOLL_SIZE); /* 保证描述符在范围内 */

	bool			flag = false;
	struct comm_message	*commmsg = NULL;
	struct comm_data	*commdata = (struct comm_data *)commctx->commevent->data[message->fd];
	if (likely(commdata)) {
		if (new_commmsg(&commmsg, message->package.dsize)){
			copy_commmsg(commmsg, message);
			commlock_lock(&commdata->sendlock);

			if (likely(commqueue_push(&commdata->send_queue, (void*)&commmsg))) {
				/* 数据保存成功 */
				flag = true;
			} else {
				commlist_push(&commctx->msghead, &commmsg->list);
				flag = true;
			} 

			commlock_unlock(&commdata->sendlock);

			if (likely(flag)) {
				/* 数据写入成功 往此管道写入fd 触发子线程的写事件 */
				commpipe_write(&commctx->commpipe, (void*)&message->fd, sizeof(message->fd));
				log("comm_send data in comm_send\n");
				return message->package.dsize;
			} else {
				free_commmsg(commmsg);		/* push数据失败则需要释放commmsg */
				return -1;
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
	do {
		if (likely(commqueue_pull(&commctx->recvqueue, (void*)&commmsg))) {
			/* 数据成功获取 */
			break ;
		} else if (block) {
			commctx->recvqueue.readable = 0;
			if (commlock_wait(&commctx->recvlock, &commctx->recvqueue.readable, 1, timeout, true)) {
				flag = true;			/* 返回值为真说明有数据可读 */
			}
		} else {
			commctx->recvqueue.readable = 1;	/* 不进行堵塞就设置为1不需要唤醒 */
		}
	}while(flag);						/* flag为true说明成功等待到数据 尝试再去取一次数据 */

	commlock_unlock(&commctx->recvlock);

	if (likely(commmsg)) {					/* 取到数据 */
		copy_commmsg(message, commmsg);
		free_commmsg(commmsg);				/* 释放掉comm_message结构体 */
		return message->package.dsize;
	} else {
		return -1;
	}
}

void comm_settimeout(struct comm_context *commctx, int timeout, CommCB callback, void *usr)
{
	commctx->commevent->timeoutcb.timeout = timeout;
	commctx->commevent->timeoutcb.callback  = callback;
	commctx->commevent->timeoutcb.usr = usr;
}

void comm_close(struct comm_context *commctx, int fd)
{
	commevent_del(commctx->commevent, fd);
	close(fd);
	return ;
}

/* 一个新的线程开始运行 */
static void * _start_new_pthread(void* usr)
{
	assert(usr);
	int			n = 0;
	int			fdidx = 0;	/* 监听fd在结构体struct listenfd中数组的下标 */
	bool			retval = false;
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
		if (commctx->commevent->timeoutcb.timeout <= 0) {
			commctx->commevent->timeoutcb.timeout = TIMEOUTED;
		}

		if ((retval = commepoll_wait(&commctx->commepoll, commctx->commevent->timeoutcb.timeout))) {
			for (n = 0; n < commctx->commepoll.eventcnt; n++) {
				if (likely(commctx->commepoll.events[n].data.fd > 0)) {
					if ((fdidx = gain_listenfd_fdidx(&commctx->commevent->listenfd, commctx->commepoll.events[n].data.fd)) > -1) {	/* 新用户连接，触发accept事件 */
						 commevent_accept(commctx->commevent, fdidx);

					} else if (commctx->commepoll.events[n].events & EPOLLIN) {					/* 有数据可读，触发读数据事件 */
					
						if (commctx->commepoll.events[n].data.fd == commctx->commpipe.rfd) {			/* 管道事件被触发， 开始写事件 */
							int	cnt = 0;
							int	array[EPOLL_SIZE] = {};
#if 0
							struct remainfd* remainfd = &commctx->commevent->remainfd;
							if ((cnt = commpipe_read(&commctx->commpipe, &remainfd->wfda[remainfd->wcnt], sizeof(int))) > 0) {
								remainfd->wcnt += cnt;
								commevent_send(commctx->commevent, remainfd->wfda[remainfd->wcnt-1], true);
							}
#endif
							if ((cnt = commpipe_read(&commctx->commpipe, array, sizeof(int))) > 0) {
								struct remainfd* remainfd = &commctx->commevent->remainfd;
								//remainfd->wcnt += cnt;
								if (_set_remainfd(remainfd, array, cnt)) {
									commevent_send(commctx->commevent, remainfd->wpfda[remainfd->wpcnt-1], true);
								}
							}
						} else {
							 commevent_recv(commctx->commevent, commctx->commepoll.events[n].data.fd, false);
						}
					} else if (commctx->commepoll.events[n].events & EPOLLOUT) {					/* 有数据可写， 触发写数据事件 */

						commevent_send(commctx->commevent, commctx->commepoll.events[n].data.fd, false);
					}
				}
			}
		}
		if (retval == false && errno == EINTR) {
			/* epoll_wait超时调用处理残留fd函数 */
			commevent_remainfd(commctx->commevent, true);
		} else {
			/* epoll_wait每次循环完一次就去处理一次残留的fd */
			commevent_remainfd(commctx->commevent, false);
		}
	}
	return NULL;
}

static bool _set_remainfd(struct remainfd *remainfd, int array[], int n)
{
	int i =0;
	if (_quick_sort(array, n)) {
		for (i = 0; i < n; i++) {
			if (array[i] != array[i+1]) {
				remainfd->wpfda[remainfd->wpcnt++] = array[i];
			}
		}
		return true;
	}
	return false;
}

static bool _quick_sort(int array[], int n)
{
	struct stack {
		int start;	/* 数组中第一个元素下标 */
		int end;	/* 数组中最后一个元素下标*/
	};

	int index= 0;				/* 栈的下标 */ 
	int size = sizeof(struct stack);	/* struct stack结构体大小 */
	int i = 0, j = 0, key = 0;
	int left = 0, right = 0;
	struct stack *stack = NULL;
	char* memory = calloc(n, size);
	
	if (memory) {
		stack = (struct stack*)&(memory[index*size]);
		stack->start = 0;
		stack->end = n -1;
		while (index > -1) {
			i = left = stack->start;	/* 数组从左开始数据的下标 */
			j = right = stack->end;		/* 数组从右开始数据的下标 */
			key = array[left];
			index --;
			while (i<j) {
				while ((i<j) && (key <= array[j])) {j--;}
				if (i < j) {
					array[i] = array[i] ^ array[j];
					array[j] = array[i] ^ array[j];
					array[i] = array[i] ^ array[j];
					i++;
				}

				while ((i<j) && (key >= array[i])) {i++;}
				if (i < j) {
					array[i] = array[i] ^ array[j];
					array[j] = array[i] ^ array[j];
					array[i] = array[i] ^ array[j];
					j--;
				} 
			}//处理一次  即将比绑定值小的全部放左边  比绑定值大的放右边

			if (left < i-1) {
				index++;
				stack = (struct stack*)&memory[index*size];
				stack->start = left;
				stack->end = i-1;
			}
			if (right>i+1) {
				index++;
				stack = (struct stack*)&memory[index*size];
				stack->start = i+1;
				stack->end = right;
			}
			stack = (struct stack*)&memory[index*size];
		}
		free(memory);
		return true;
	}
	return false;
}
