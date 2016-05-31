/*********************************************************************************************/
/************************	Created by 许莉 on 16/02/25.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include <dirent.h>
#include <stdlib.h>
#include "communication.h"

#define	QUEUE_NODES	1024	/* 队列里面可以存放多少个节点的数据 */
#define TIMEOUTED	5000	/* 以毫秒(ms)为单位 1s = 1000ms*/

static void * _start_new_pthread(void *usr);

static bool _set_remainfd(struct remainfd *remainfd, int array[], int n);

static bool _quick_sort(int array[], int n);

struct comm_context* comm_ctx_create(int epollsize)
{
	struct comm_context* commctx = NULL;

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

	if (unlikely(!commpipe_init(&commctx->commpipe))) {
		goto error;
	}

	if (unlikely(!commepoll_init(&commctx->commepoll, (epollsize > 0 && epollsize <= EPOLL_SIZE) ? epollsize : EPOLL_SIZE))) {
		goto error;
	}

	if (unlikely(!commepoll_add(&commctx->commepoll, commctx->commpipe.rfd, EPOLLET | EPOLLIN))) {
		goto error;
	}

	if (unlikely((pthread_create(&commctx->ptid, NULL, _start_new_pthread, (void*)commctx))) != 0) {
		goto error;
	}
	commlist_init(&commctx->recvlist, free_commmsg);
	commctx->stat = COMM_STAT_INIT; 

	return commctx;
error:
	if (commctx) {
		commqueue_destroy(&commctx->recvqueue);
		commlock_destroy(&commctx->statlock);
		commlock_destroy(&commctx->recvlock);
		commpipe_destroy(&commctx->commpipe);
		if (commctx->commepoll.init && commctx->commepoll.watchcnt > 0) {
			/* 如果pipe的fd被监听成功则删除pipe的fd监控 */
			commepoll_del(&commctx->commepoll, commctx->commpipe.rfd, -1);
		}
		commevent_destroy(commctx->commevent);
		commepoll_destroy(&commctx->commepoll);
		commlist_destroy(&commctx->recvlist, COMMMSG_OFFSET);
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

		commevent_destroy(commctx->commevent);
		commqueue_destroy(&commctx->recvqueue);
		commlock_destroy(&commctx->statlock);
		commlock_destroy(&commctx->recvlock);

		/* 删除pipe的fd的监控 */
		commepoll_del(&commctx->commepoll, commctx->commpipe.rfd, -1);
		commpipe_destroy(&commctx->commpipe);
		commlist_destroy(&commctx->recvlist, COMMMSG_OFFSET);
		commepoll_destroy(&commctx->commepoll);
		pthread_join(commctx->ptid, NULL);
		Free(commctx);
	}
	return ;
}


int comm_socket(struct comm_context *commctx, const char *host, const char *service, struct cbinfo *finishedcb, int type)
{
	assert(commctx && host && service);

	struct comm_tcp	  commtcp = {};
	struct comm_data* commdata = NULL;

	if (type == COMM_BIND) {
		if (unlikely(!socket_listen(&commtcp, host, service))) {
			return -1;
		}
	} else {
		if (unlikely(!socket_connect(&commtcp, host, service))) {
			return -1;
		}
	}

	/* 添加一个fd进行监听 */
	if (unlikely(!commevent_add(commctx->commevent, &commtcp, finishedcb))) {
		close(commtcp.fd);
		return -1;
	}

	/* 将状态值设置为COMM_STAT_RUN并唤醒等待的线程 */
	commlock_wake(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_RUN, false);
	
	return commtcp.fd;
}

/* 实际发送数据无需堵塞，@block @timeout为保留参数 目前没有使用 */
int comm_send(struct comm_context *commctx, const struct comm_message *message, bool block, int timeout)
{
	assert(commctx && message && message->content);
	assert(message->fd > 0 && message->fd < EPOLL_SIZE); /* 保证描述符在范围内 */

	struct comm_data*    commdata = NULL;
	struct comm_message* commmsg = NULL;

	if ((commdata = (struct comm_data*)commctx->commevent->data[message->fd])) {
		if (new_commmsg(&commmsg, message->package.dsize)){
			copy_commmsg(commmsg, message);

			commlock_lock(&commdata->sendlock);
			if (unlikely(!commqueue_push(&commdata->send_queue, (void*)&commmsg))) {
				/* 队列已满，则存放到链表中 */
				commlist_push(&commdata->send_list, &commmsg->list);
			}
			commlock_unlock(&commdata->sendlock);

			commpipe_write(&commctx->commpipe, (void*)&message->fd, sizeof(message->fd));
			//log("comm_send data in comm_send\n");
			return message->package.dsize;
		} 
	}
	return -1;
}

int comm_recv(struct comm_context *commctx, struct comm_message *message, bool block, int timeout)
{
	assert(commctx && message && message->content);

	bool flag = false;
	struct comm_message* commmsg = NULL;

	commlock_lock(&commctx->recvlock);
	do {
		if (unlikely(!commqueue_pull(&commctx->recvqueue, (void*)&commmsg))) {
			struct comm_list* list = NULL;
			if (commlist_pull(&commctx->recvlist, (void*)&list)) {
				commmsg = (struct comm_message*)get_container_addr(list, COMMMSG_OFFSET);
				break ;
			} else if (block) {
				commctx->recvqueue.readable = 0;
				if (commlock_wait(&commctx->recvlock, &commctx->recvqueue.readable, 1, timeout, true)) {
					flag = true;			/* 返回值为真说明有数据可读 */
				}
			} else {
				commctx->recvqueue.readable = 1;	/* 不进行堵塞就设置为1不需要唤醒 */
			};
		} else {
			break ;
		}
	}while(flag);							/* flag为true说明成功等待到数据 尝试再去取一次数据 */
	commlock_unlock(&commctx->recvlock);

	if (commmsg) {					/* 取到数据 */
		copy_commmsg(message, commmsg);
		free_commmsg(commmsg);				/* 释放掉comm_message结构体 */
		return message->package.dsize;
	}

	return -1;
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
static void * _start_new_pthread(void *usr)
{

	struct comm_context* commctx = (struct comm_context*)usr;
	assert(commctx);

	int  n = 0;
	int  fdidx = 0;		/* 监听fd在结构体struct listenfd中数组的下标 */
	bool retval = false;

	/* 等待主线程将状态设置为COMM_STAT_RUN，才被唤醒继续执行以下代码 */
	commlock_wait(&commctx->statlock, (int*)&commctx->stat, COMM_STAT_RUN, -1, false);

	while (1) {
		/* 线程状态为STOP的时候则将状态设置为NONE，返回真，则代表设置成功，退出循环 */
		if (unlikely(commctx->stat == COMM_STAT_STOP)) {
			commlock_wake(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_NONE, false);
			break;
		}					
		/* 用户没有设置epoll_wait超时，则使用默认超时时间 */
		if (commctx->commevent->timeoutcb.timeout < 1) {
			commctx->commevent->timeoutcb.timeout = TIMEOUTED;
		}
		/* 开始epoll_wait等待描述符就绪 */
		if ((retval = commepoll_wait(&commctx->commepoll, commctx->commevent->timeoutcb.timeout))) {
			for (n = 0; n < commctx->commepoll.eventcnt; n++) {
				if (commctx->commepoll.events[n].data.fd > 0) {
					/* 新的客户端连接， 触发accept事件 */
					if ((fdidx = gain_listenfd_fdidx(&commctx->commevent->listenfd, commctx->commepoll.events[n].data.fd)) > -1) {
						 commevent_accept(commctx->commevent, fdidx);

					} else if (commctx->commepoll.events[n].events & EPOLLIN) {			/* 有数据可读，触发读数据事件 */
						/* 管道事件被触发，则开始写事件 */
						if (commctx->commepoll.events[n].data.fd == commctx->commpipe.rfd) {
							int cnt = 0;
							int array[EPOLL_SIZE] = {};
							if ((cnt = commpipe_read(&commctx->commpipe, array, sizeof(int))) > 0) {
							//	int i = 0;
								//struct remainfd* remainfd = &commctx->commevent->remainfd;
								_set_remainfd(&commctx->commevent->remainfd, array, cnt);
#if 0
								for(i = 0; i < cnt; i++) {
									add_remainfd(&commctx->commevent->remainfd, array[i], REMAINFD_WRITE);
								}
								/* 将从管道中读取到的重复的fd去除，只保留一个，即相同的fd只触发一次写事件 */
								if (_set_remainfd(remainfd, array, cnt)) {
									commevent_send(commctx->commevent, remainfd->wfda[remainfd->wcnt-1], true);
								}
#endif
							}
						} else {	/* 非pipe的fd读事件被触发，则代表是socket的fd触发了读事件 */
							 commevent_recv(commctx->commevent, commctx->commepoll.events[n].data.fd, false);
						}
					} else if (commctx->commepoll.events[n].events & EPOLLOUT) {			/* 有数据可写， 触发写数据事件 */

						commevent_send(commctx->commevent, commctx->commepoll.events[n].data.fd, false);
					}
				}
			}
		}
		if (retval == false && errno == EINTR) {
			/* epoll_wait超时调用处理残留fd函数 @true会调用用户的超时回调函数 */
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
	for(i = 0; i < n; i++) {
		add_remainfd(remainfd, array[i], REMAINFD_WRITE);
	}
#if 0
	if (_quick_sort(array, n)) {
		for (i = 0; i < n; i++) {
			if (array[i] != array[i+1]) {
				remainfd->wfda[remainfd->wcnt++] = array[i];
			}
		}
		return true;
	}
#endif
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
			}

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
