/*********************************************************************************************/
/************************	Created by 许莉 on 16/02/25.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include <dirent.h>
#include <stdlib.h>
#include "communication.h"

#define MIOU	1024

static void * _start_new_pthread(void* usr);

static inline bool _get_portinfo(int fd, int type, struct portinfo* portinfo);

static bool  _close_all_fd(int pfd);

static bool _parse_data(struct comm_data *commdata);

static bool _package_data(struct comm_data *commdata);

static void _timeout_event(struct comm_context* commctx);

static bool _accept_event(struct comm_context *commctx);

static bool _read_event(struct comm_data *commdata, int fd);

static bool _write_event(struct comm_data *commdata, int fd);

struct comm_context* comm_ctx_create(int epollsize)
{
	int			retval = -1;
	struct comm_context*	commctx = NULL;

	New(commctx);
	if (unlikely(!commctx)) {
		printf("no more memory, calloc commctx failed\n");
		return commctx;
	}

	commctx->epfd = create_epoll(epollsize > 0 ? epollsize : EPOLL_SIZE);
	if (unlikely(commctx->epfd < 0)) {
		Free(commctx);
		return commctx;
	}

	if (unlikely( !commqueue_init(&commctx->recv_queue, QUEUE_CAPACITY, sizeof(intptr_t)))) {
		close(commctx->epfd);
		Free(commctx);
		return commctx;
	}

	if (unlikely(!commlock_init(&commctx->commlock))) {
		close(commctx->epfd);
		commqueue_destroy(&commctx->recv_queue);
		Free(commctx);
		return commctx;
	}

	commctx->listenfd = -1;
	retval = pthread_create(&commctx->ptid, NULL,  _start_new_pthread, (void *)commctx);
	if (unlikely(retval != 0)) {
		close(commctx->epfd);
		commqueue_destroy(&commctx->recv_queue);
		Free(commctx);
		return commctx;
	}
	commctx->stat = COMM_STAT_INIT; 

	return commctx;
}

void comm_ctx_destroy(struct comm_context* commctx)
{
	int fd = 0;
	if (likely(commctx)) {
		//原子设置线程的状态
		ATOMIC_SET(&commctx->stat, COMM_STAT_STOP);
		commlock_wait(&commctx->commlock, (int *)&commctx->stat, COMM_STAT_NONE, -1);
		while (commctx->watchcnt) {
			if(likely(commctx->data[fd ])) {
				comm_close(commctx, fd);
			}
			fd ++;
		}
		close(commctx->epfd);
		commqueue_destroy(&commctx->recv_queue);
		pthread_join(commctx->ptid, NULL);
		Free(commctx);
	}
}

int comm_socket(struct comm_context *commctx, char *host, char *service, struct cbinfo* finishedcb, int type)
{
	assert(commctx && host && service);

	int			fd = -1;
	bool			retval = false;
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

	struct portinfo portinfo = {};

	if (unlikely(!_get_portinfo(fd, type, &portinfo))) {
		close(fd);
		return -1;
	}
	commdata = commdata_init(commctx, &portinfo, finishedcb);
	if (unlikely(!commctx)) {
		close(fd);
		return -1;
	}
	commctx->data[fd ] = (intptr_t)commdata;
	commlock_wake(&commctx->commlock, (int *)&commctx->stat, COMM_STAT_RUN);
	
	return fd;
}



/* 将用户传进来的数据放入到发送队列中保存起来:返回-1失败,成功返回存入的数据的大小 */
int comm_send(struct comm_context *commctx, const struct comm_message *message)
{
	assert(commctx && message && message->content);

	struct comm_message *commmsg = NULL;
	commmsg = new_commmsg(message->size); /* 分配一个comm_message的结构体 */
	if (unlikely(!commmsg)) {
		return -1;
	}
	copy_commmsg(commmsg, message);
	struct comm_data *commdata = (struct comm_data*)commctx->data[message->fd];
	if (unlikely(commdata)) {
		if (unlikely(!commqueue_push(&commdata->send_queue, commmsg))) {
			return -1;
		}
	}
	return message->size;
}

/* 将已经解析好的数据返回给用户:返回-1失败,成功返回数据的大小 */
int comm_recv(struct comm_context *commctx, struct comm_message *message)
{
	assert(commctx && message && message->content);

	struct comm_message *commmsg = NULL;
	if (unlikely(!commqueue_pull(&commctx->recv_queue, commmsg))) {
		return -1;
	}
	copy_commmsg(message, commmsg);
	free_commmsg(commmsg); /* 释放掉comm_message结构体 */

	return commmsg->size;
}

/* 设置epoll_wait的超时时间以及超时时的回调函数 */
void comm_settimeout(struct comm_context *commctx, int timeout, CommCB callback, void *usr)
{
	commctx->timeoutcb.timeout = timeout;
	commctx->timeoutcb.callback  = callback;
	commctx->timeoutcb.usr = usr;
}

/* 关闭指定的描述符 */
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


static inline bool _get_portinfo(int fd, int type, struct portinfo *portinfo)
{
	int	retval = -1;

	retval = get_address(fd,  portinfo->addr, (size_t)sizeof(portinfo->addr));
	if (unlikely(retval == -1)) {
		return false;
	}
	portinfo->port = get_port(fd);
	if (unlikely(portinfo->port ==  -1)) {
		return false;
	}
	portinfo->fd = fd;
	portinfo->type = type;

	return true;
}


//epoll_wait超时时的回调函数:解析和打包数据
static void _timeout_event(struct comm_context* commctx)
{
	int fd = 0;
	int counter = commctx->watchcnt;
	while (counter) {
		struct comm_data *commdata = (struct comm_data*)commctx->data[fd];
		if (likely(commdata)) {
			//接收缓冲区里面存在数据需要解析
			if (likely(commdata->recv_buff.end - commdata->recv_buff.start > 0)) {

				_parse_data(commdata);
			}
			//发送缓冲区里面存在数据需要打包
			if (likely(commdata->send_buff.end - commdata->recv_buff.start > 0)) {
				_package_data(commdata);
			}
			counter--;
		}
		fd++;
	}
	if (likely(commctx->timeoutcb.callback)) { //调用用户层的回调函数
		commctx->timeoutcb.callback(commctx, -1, FD_TIMEOUT, commctx->timeoutcb.usr);
	}
}

/* 一个新的线程开始运行 */
static void * _start_new_pthread(void* usr)
{
	assert(usr);
	bool			retval = false;
	struct comm_context*	commctx = (struct comm_context*)usr;
#if 0
	if (unlikely(!(retval = _close_all_fd(commctx->epfd)))) {
		return NULL;
	}
#endif

	commlock_wait(&commctx->commlock, (int*)&commctx->stat, COMM_STAT_RUN, -1);
	while (1) {
		//线程状态为STOP的时候则，将状态设置为NONE，返回真，则代表设置成功，退出循环
		if (unlikely(ATOMIC_CASB(&commctx->stat, COMM_STAT_STOP, COMM_STAT_NONE))) {
			break;
		}
		if (commctx->timeoutcb.timeout <= 0) { //用户没有设置epoll_wait超时，则使用默认超时时间
			commctx->timeoutcb.timeout = TIMEOUTED;
		}
		struct epoll_event	events[EPOLL_SIZE] = {};
		int			nfds = -1;
		int			n = 0;
		nfds = epoll_wait(commctx->epfd, events, commctx->watchcnt, commctx->timeoutcb.timeout);
		if (likely(nfds > 0)) {
			for (n = 0; n < nfds; n++) {
				if (unlikely(events[n].data.fd < -1)) {
					continue ;
				}
				if (commctx->listenfd != -1 && events[n].data.fd == commctx->listenfd) {
					/* 新用户连接，触发accept事件 */
					retval = _accept_event(commctx);
					if (likely(retval)) {
						continue ;
					} else {
						/* 返回false */
						//return NULL;
					}
				} else if (events[n].events & EPOLLIN) {
					/* 有数据可读，触发读数据事件 */
					retval = _read_event((struct comm_data*)commctx->data[events[n].data.fd], events[n].data.fd);
					if (likely(retval)) {
						continue ;
					} else {
						/* 返回false */
					}
				} else if (events[n].events & EPOLLOUT) {
					/* 有数据可写， 触发写数据事件 */
					retval = _write_event((struct comm_data*)commctx->data[events[n].data.fd], events[n].data.fd);
					if (likely(retval)) {
						continue ;
					} else {
						/* 返回false */
					}
					
				}
			}
		} else {
			//epoll_wait 超时
			if (likely(errno == EINTR)) {
				if(commctx->timeoutcb.timeout > 0){
					_timeout_event(commctx);
				}
			}
		}
	}
	return NULL;
}

/* 接收新用户的连接 */
static bool _accept_event(struct comm_context *commctx)
{
	int fd = -1;
	while (1) {
		fd = accept(commctx->listenfd, NULL, NULL);
		if (likely(fd > 0)) {
			if (unlikely(!fd_setopt(fd, O_NONBLOCK))) {
				close(fd);
				continue ;
			}
				
			struct portinfo portinfo = {};
			if (unlikely(!_get_portinfo(fd, COMM_ACCEPT, &portinfo))) {
				close(fd);
				continue ;
			}
			struct comm_data* commdata = NULL;
			commdata = commdata_init(commctx, &portinfo, NULL);
			if (unlikely(!commdata)) {
				close(fd); //无法对此描述符进行监控，则关闭此描述符
				 continue;
			} else {
				commctx->data[fd ] = (intptr_t)commdata;
				struct comm_data *lsnfd_commdata = (struct comm_data*)commctx->data[commctx->listenfd];
				if (lsnfd_commdata->finishedcb.callback) {
					lsnfd_commdata->finishedcb.callback(lsnfd_commdata->commctx, fd, FD_ACCEPT, lsnfd_commdata->finishedcb.usr);
				}
				continue ;
			}
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				/* 对于连接的用户已经全部进行处理 */
				return true;
			} else if (likely(errno == ECONNABORTED || errno == EPROTO || errno == EINTR )) {
				continue ;
			} else {
				return false;
			}
		}
	}
	return true;
}


/* 有数据可以读的时候 */
static bool _read_event(struct comm_data *commdata, int fd)
{
	int	bytes = 0;
	int	status = FD_READ;
	bool	flag = false;

	while (1) {
		bytes = read(fd, &commdata->recv_buff.cache[commdata->recv_buff.end], COMM_READ_MIOU);
		if (unlikely(bytes < 0)) {
			if (likely(errno == EAGAIN || errno == EWOULDBLOCK)) {
				/* 说明数据已经全部接收完毕 */
				flag = true;
				_parse_data(commdata);
				break ;
			} else if (likely(errno == EINTR)) {
				/* 被中断打断，继续接收数据 */
				continue ;
			} else {
				/* 其他错误 */
				flag = false;
				break ;
			}
		} else if (likely(bytes == 0)){
			/* 对端已关闭 */
			status = FD_CLOSE;
			flag = true;
			break ;
		} else {
			/* 数据接收成功 */
			commdata->recv_buff.end += bytes;
			if (bytes < COMM_READ_MIOU) {
				/* 数据已经全部读完 */
				flag = true;
				_parse_data(commdata);
				break ;
			}
		}
	}

	if (commdata->finishedcb.callback) {
		if (unlikely(status == FD_CLOSE)) {
			commdata->finishedcb.callback(commdata->commctx, fd, status, commdata->finishedcb.usr);
			comm_close(commdata->commctx, fd);
		} else {
			commdata->finishedcb.callback(commdata->commctx, fd, status, commdata->finishedcb.usr);
		}
	}

	return flag;
}

/* 有数据可以写的时候 */
static bool _write_event(struct comm_data *commdata, int fd)
{
	int bytes = 0;
	int flag = false;
	int size = commdata->send_buff.size;
	while (1) {
		bytes = write(fd, &commdata->send_buff.cache[commdata->send_buff.start], size);
		if (unlikely(bytes < 0)) {
			if (likely(errno == EAGAIN || errno == EWOULDBLOCK)) {
				/* 写缓冲区列队已满 */
				flag = false ;
				_package_data(commdata);
				break ; 
			} else if (likely(errno == EINTR)) {
				/* 写操作被信号中断 可继续写 */
				continue ;
			} else {
				/* 其他错误，退出 */
				flag = false;
				break ;
			}

		} else {
			/* 数据成功发送完成 */
			flag = true;
			commdata->send_buff.start += bytes;
			size -= bytes;
			if (size == bytes) {
				/* 发送完毕，退出循环 */
				commcache_clean(&commdata->send_buff); /* 待定中 */
				/* 接收完数据就打包 */
				_package_data(commdata);
				break ;
			}
		}
	}
	if (commdata->finishedcb.callback) {
		commdata->finishedcb.callback(commdata->commctx, fd, FD_WRITE, commdata->finishedcb.usr);
	}

	return flag;
}

/* 模拟解析函数 返回值为解析了多少大小的数据 */
int parse(const char* buff)
{
	/* 真正的解析数据在解析结构体里面需要告知body的偏移 */
	int size = rand()%10;
	return size;
}

/* 解析数据 */
static bool _parse_data(struct comm_data *commdata)
{
	/* 测试用，只是单纯将recv_buff里面的数据拷贝到recv_queue中 */
	bool			flag = false;
	int			size = 0; /* 成功解析数据的字节数 */
	struct comm_context	*commctx = commdata->commctx;
	struct comm_message	*message = new_commmsg(size);

	size = parse(&commdata->recv_buff.cache[commdata->recv_buff.start]);
	if (!message) {
		return false;
	}
	memcpy(message->content, &commdata->recv_buff.cache[commdata->recv_buff.start], size);
	message->fd = commdata->portinfo.fd;
	message->size = size;
	commdata->recv_buff.start += size;
	commqueue_push(&commdata->recv_queue, message);
	commcache_clean(&commdata->recv_buff);
	return flag;
}


/* 模拟打包数据 返回值为打包之后数据的大小 */
int  package(const char *buff, int size, char*packbuff)
{
	memcpy(packbuff, buff, size);
	return size;
}

/* 打包数据 */
static bool _package_data(struct comm_data *commdata)
{
	/* 测试用，只是单纯的额将send_queue里面的数据拷贝到send_buff里面 */
	bool			flag = false;
	int			 packsize = 0; /* 打包之后的数据大小 */
	struct comm_context	*commctx = commdata->commctx;
	struct comm_message	*message = NULL;

	commqueue_pull(&commdata->send_queue, message);
	packsize = package(message->content, message->size, &commdata->send_buff.cache[commdata->send_buff.end]);
	commdata->send_buff.end += packsize;
	free_commmsg(message);
	return flag;
}


/*关闭父进程所有打开的文件描述符， pfd:此描述符不关闭*/
static bool  _close_all_fd(int pfd)
{
	int		retval = 0;
	int		rewind = 0;
	int		fd = 0;
	DIR*		dir = NULL;
	struct dirent	*entry, _entry;

	if (unlikely(!(dir = opendir("/dev/fd")))) {
		return false;
	}

	while (1) {

		retval = readdir_r(dir, &_entry, &entry);
		if (unlikely(retval != 0)) {
			closedir(dir);
			return false;
		}
		if (entry == NULL) {
			if (!rewind) {
				break;
			}
			rewinddir(dir);
			rewind = 0;
			continue;
		}
		if (entry->d_name[0] == '.') {
			continue;
		}
		fd = atoi(entry->d_name);
		if (unlikely(dirfd(dir) == fd || fd == 1 || fd == 0 ||fd == 2 || fd == pfd)) {
			continue;
		}
		close(fd);
		rewind = 1;
	}
	closedir(dir);
	return true;
}
