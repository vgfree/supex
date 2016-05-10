/*********************************************************************************************/
/************************	Created by 许莉 on 16/02/25.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include <dirent.h>
#include <stdlib.h>

//#include "loger.h"
#include "communication.h"


static void * _start_new_pthread(void* usr);

static inline bool _get_portinfo(int fd, int type, int status, struct portinfo* portinfo);

static bool _parse_data(struct comm_data *commdata);

static bool _package_data(struct comm_data *commdata);

static void _timeout_event(struct comm_context* commctx);

static void _accept_event(struct comm_context *commctx);

static void _recv_event(struct comm_context *commctx, int fd);

static void  _send_event(struct comm_context *commctx, int *fda, int cnt);

static bool _write_data(struct comm_data *commdata, int fd);

static bool _read_data(struct comm_data *commdata, int fd);

static void _fill_message_package(struct comm_message *message, const struct mfptp_package_info *package); 

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
		commlock_wait(&commctx->statlock, (int *)&commctx->stat, COMM_STAT_NONE, -1, false);

		while (commctx->commepoll.watchcnt) {
			if(likely(commctx->data[fd ])) {
				comm_close(commctx, fd);
				log("close fd:%d in comm_ctx_destroy\n", fd);
			}
			fd ++;
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

	if (unlikely(!_get_portinfo(fd, type, FD_INIT, &portinfo))) {
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
		if (unlikely(ATOMIC_CASB(&commctx->stat, COMM_STAT_STOP, COMM_STAT_NONE))) {
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
					 _accept_event(commctx);

				} else if (commctx->commepoll.events[n].events & EPOLLIN) {					/* 有数据可读，触发读数据事件 */
				
					if (commctx->commepoll.events[n].data.fd == commctx->commpipe.rfd) {			/* 管道事件被触发， 开始写事件 */
						int array[EPOLL_SIZE] = {0};
						int cnt = commpipe_read(&commctx->commpipe, array, sizeof(int));
						if (likely(cnt > 0)) {
							 _send_event(commctx, array, cnt);
						} else {
							continue ;
						}
					} else {
						 _recv_event(commctx, commctx->commepoll.events[n].data.fd);
					}
				} else if (commctx->commepoll.events[n].events & EPOLLOUT) {					/* 有数据可写， 触发写数据事件 */
					
					int array[1] = {commctx->commepoll.events[n].data.fd};
					 _send_event(commctx, array, 1);
				}
			}
		} else {	
			if (likely(errno == EINTR)) {		/* epoll 超时 */
				if(commctx->timeoutcb.timeout > 0){
					_timeout_event(commctx);
				}
			}
		}
	}
	return NULL;
}

static inline bool _get_portinfo(int fd, int type, int status, struct portinfo *portinfo)
{
	assert(portinfo && portinfo->addr);
	if (unlikely((get_address(fd, portinfo->addr, (size_t)sizeof(portinfo->addr))) == -1)) {
		return false;
	}
	portinfo->port = get_port(fd);
	if (unlikely(portinfo->port ==  -1)) {
		return false;
	}
	portinfo->fd = fd;
	portinfo->type = type;
	portinfo->stat = status;

	return true;
}

/* epoll_wait超时时的回调函数:解析和打包数据 */
static void _timeout_event(struct comm_context* commctx)
{
	int fd = 0;
	int counter = commctx->commepoll.watchcnt;
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
		commctx->timeoutcb.callback(commctx, NULL, commctx->timeoutcb.usr);
	}
}

/* 接收新用户的连接 */
static void  _accept_event(struct comm_context *commctx)
{
	assert(commctx);
	int fd = -1;
	struct comm_data *lsnfd_commdata = (struct comm_data*)commctx->data[commctx->listenfd];

	while (1) {

		struct portinfo		portinfo = {};
		struct comm_data*	commdata = NULL;
		fd = accept(commctx->listenfd, NULL, NULL);
		if (likely(fd > 0)) {
			if (unlikely(!fd_setopt(fd, O_NONBLOCK))) {
				close(fd);
				continue ;
			}
				
			if (unlikely(!_get_portinfo(fd, COMM_ACCEPT, FD_INIT, &portinfo))) {
				close(fd);
				continue ;
			}
			commdata = commdata_init(commctx, &portinfo, &lsnfd_commdata->finishedcb);
			if (likely(commdata)) {
				commctx->data[fd ] = (intptr_t)commdata;
				if (lsnfd_commdata->finishedcb.callback) {
					lsnfd_commdata->finishedcb.callback(lsnfd_commdata->commctx, &portinfo, lsnfd_commdata->finishedcb.usr);
				}
			} else {
				close(fd);
			}
		} else {
			if (unlikely(errno == ECONNABORTED || errno == EPROTO || errno == EINTR )) {
				/* 可能被打断，继续处理下一个连接 */
				continue ;
			} else {
				/* 处理完毕或者出现致命错误，则直接返回 */
				return ;
			}
		}
	}
}


/* 有数据可以接收的时候 */
static void _recv_event(struct comm_context *commctx, int fd)
{
	assert(commctx);
	int			n = 0;
	int			bytes = 0;
	struct comm_data*	commdata = NULL;

	/* 先处理残留下来没处理完的fd */
	for (n = 0; n < commctx->remainfd.rcnt; n++) {
		commdata = (struct comm_data*)commctx->data[commctx->remainfd.rfda[n]];
		if (likely(commdata)) {
			if (likely( _read_data(commdata, commctx->remainfd.rfda[n]))) {
				/* 发送数据成功 */
				commctx->remainfd.rcnt -= 1;
				_parse_data(commdata); /* 目前暂定只要接收到数据就解析 */
			} else {
				/* 发送数据失败*/
				commctx->remainfd.rfda[commctx->remainfd.rcnt-1] = commctx->remainfd.rfda[n];
				commctx->remainfd.rcnt += 1;
			}
		} else {
			commctx->remainfd.rcnt -= 1;
		}
	}

	/* 开始处理新的fd */
	commdata = (struct comm_data*)commctx->data[fd];
	if (likely(commdata)) {
		if (likely(_read_data(commdata, fd))) {
			/* 发送数据成功没有被关闭 */
			commdata = (struct comm_data*)commctx->data[fd];
			if (commdata) {
				_parse_data(commdata); /* 目前暂定只要接收到数据就是解析 */
			}
		} else {
			/* 数据发送失败 */
			commctx->remainfd.rfda[commctx->remainfd.rcnt-1] = commctx->remainfd.rfda[n];
			commctx->remainfd.rcnt += 1;
		}
	}
	return ;
}

/* 有数据可以发送的时候 */
static void  _send_event(struct comm_context *commctx, int* fda, int cnt)
{
	assert(commctx && fda);
	int			n = 0;
	int			bytes = 0;
	struct comm_data*	commdata = NULL;

	/* 先处理遗留的没有处理的fd */
	for (n = 0; n < commctx->remainfd.wcnt; n++) {
		commdata = (struct comm_data*)commctx->data[commctx->remainfd.wfda[n]];
		if (likely(commdata)) {
			if (likely(_write_data(commdata, commctx->remainfd.wfda[n]))) {
				/* 数据发送成功 */
				commctx->remainfd.wcnt -= 1;
			} else {
				/* 数据发送失败 将此未处理的fd保存起来 下次进行处理 */
				commctx->remainfd.wfda[commctx->remainfd.wcnt-1] = commctx->remainfd.wfda[n];
				commctx->remainfd.wcnt += 1;
			}
		} else {
			commctx->remainfd.wcnt -= 1;
		}
	}

	/* 开始处理新的fd */
	for (n = 0; n < cnt; n++) {
		commdata = (struct comm_data*)commctx->data[fda[n]];
		if (likely(commdata)) {
			if (likely(_write_data(commdata, fda[n]))) {
				/* 数据发送成功 */
			} else {
				/* 数据发送失败 将此未处理的fd保存起来 下次进行处理 */
				commctx->remainfd.wfda[commctx->remainfd.wcnt-1] = fda[n];
				commctx->remainfd.wcnt += 1;
			}
		}
	}
	return ;
}

static  bool _write_data(struct comm_data *commdata, int fd)
{
	assert(commdata);
	if (unlikely(commdata->portinfo.stat == FD_INIT)) {
		/* 第一次触发是强制触发 直接退出 */
		commdata->portinfo.stat = FD_WRITE;
		return true;
	}

	int bytes = 0;
	commdata->portinfo.stat = FD_WRITE;
	while (1) {
		if (likely(commdata->send_buff.size > 0)) {
			bytes = write(fd, &commdata->send_buff.cache[commdata->send_buff.start], commdata->send_buff.size);
			if (bytes < 0) {
				//log("write data failed fd:%d\n", fd);
				return false;
			} else {
				commdata->send_buff.start += bytes;
				commdata->send_buff.size -= bytes;
				commcache_clean(&commdata->send_buff);
				_package_data(commdata); /* 暂时每次写完数据之后就去打包一次 */
				if (commdata->finishedcb.callback) {
					commdata->finishedcb.callback(commdata->commctx, &commdata->portinfo, commdata->finishedcb.usr);
				}
				//log("write data successed fd:%d\n", fd);
				return true;
			}
		} else {
			/* @size为0则代表cache里面没数据发送，进行打包一次，然后再次尝试读取 */
			_package_data(commdata); 
			continue ;
		}
	}
}

static  bool _read_data(struct comm_data *commdata, int fd)
{
	assert(commdata);
	int bytes = 0;
	commdata->portinfo.stat = FD_READ;
	while (1) {
		bytes = read(fd, &commdata->recv_buff.cache[commdata->recv_buff.end], COMM_READ_MIOU);
		if (likely(bytes > 0)) {
			commdata->recv_buff.size += bytes;
			commdata->recv_buff.end += bytes;
			if (bytes < COMM_READ_MIOU) {		/* 数据已经读取完毕 */
				//log("read data successed\n");
				break ;
			} else {
				continue ;
			}
		} else if (bytes == 0) {			/* 对端已经关闭 */
			commdata->portinfo.stat = FD_CLOSE;
			break ;
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) { /* 数据已经读取完毕 */
				//log("read data successed\n");
				break ;
			} else {
				//log("read data failed\n");
				return false;
			}
		}
	}

	if (commdata->finishedcb.callback) {
		commdata->finishedcb.callback(commdata->commctx, &commdata->portinfo, commdata->finishedcb.usr);
	}
	if (commdata->portinfo.stat == FD_CLOSE) {
		comm_close(commdata->commctx, fd);
	}

	return true;
}

/* 解析数据 */
static bool _parse_data(struct comm_data *commdata)
{
	assert(commdata);
	bool			flag = false;
	bool			block = false;
	int			bodyoffset = 0;	/* body的偏移 */
	int			size = 0;	/* 成功解析数据的字节数 */
	int			timeout = -1;
	struct comm_context	*commctx = commdata->commctx;
	struct comm_message	*message = NULL;

	/* 有数据进行解析的时候才去进行解析 */
	if (commdata->recv_buff.size > 0) {
		size = mfptp_parse(&commdata->parser);
		if (likely(size > 0 && commdata->parser.ms.error == MFPTP_OK)) {	/* 解析成功 */
			message = new_commmsg(commdata->parser.package.dsize);
			if (likely(message)) {
				message->fd = commdata->portinfo.fd;
				memcpy(message->content, &commdata->parser.ms.cache.cache[commdata->parser.ms.cache.start], commdata->parser.package.dsize);
				_fill_message_package(message, &commdata->parser.package);
				commdata->parser.ms.cache.start += commdata->parser.package.dsize;
				commdata->parser.ms.cache.size -= commdata->parser.package.dsize;
				commdata->recv_buff.start += size;
				commdata->recv_buff.size -= size;
				commcache_clean(&commdata->recv_buff);
				commcache_clean(&commdata->parser.ms.cache);
				commlock_lock(&commctx->recvlock);
				while (1) {
					if (likely(commqueue_push(&commctx->recv_queue, (void*)&message))) {
						if (unlikely(!commctx->recv_queue.readable)) {			/* 为0代表有线程在等待可读 */
							commlock_wake(&commctx->recvlock, &commctx->recv_queue.readable, 1, true);
						}
						flag = true;
						break ;
					} else {
						/* 解析数据的时候队列已满，默认堵塞一下等待用户取数据 */
						commctx->recv_queue.writeable = 0;
						if (likely(commlock_wait(&commctx->recvlock, &commctx->recv_queue.writeable, 1, timeout, true))) {
							block = true;
							continue ;
						} else {
							break ;
						}
					}
				}
				if (unlikely(block && commctx->recv_queue.nodes == commctx->recv_queue.capacity)) {
					commctx->recv_queue.writeable = 0;
				}
				commlock_unlock(&commctx->recvlock);
			//	log("parse successed\n");
			} 
		} else if (commdata->parser.ms.error != MFPTP_DATA_TOOFEW) {
			/* 解析出错 抛弃已解析的错误数据  */
			commdata->recv_buff.start += size;
			commdata->recv_buff.size -= size;
			commcache_clean(&commdata->recv_buff);
		//	log("parse failed\n");
		}
	}
	return flag;
}

/* 打包数据 */
static bool _package_data(struct comm_data *commdata)
{
	bool			flag = false;
	bool			block = false;
	int			packsize = 0; /* 打包之后的数据大小 */
	int			timeout = -1;
	struct comm_context*	commctx = commdata->commctx;
	struct comm_message*	message = NULL;

	commlock_lock(&commdata->sendlock);
	while (1) {
		if (likely(commqueue_pull(&commdata->send_queue, (void*)&message))) {
			if (!commdata->send_queue.writeable) {
				commlock_wake(&commdata->sendlock, &commdata->send_queue.writeable, 1, true);
			}
			flag = true;
			break ;
		} else {
#if 0 /* 无数据可打包的时候 应该直接退出 */
			commdata->send_queue.readable = 0;
			if (commlock_wait(&commdata->sendlock, &commdata->send_queue.readable, 1, timeout, true)) {
				block = true;
				continue ;
			} else {
				break ;
			}
#endif
			break ;
		}
	}

	if (unlikely(block && commdata->send_queue.nodes == 0)) {
		commdata->send_queue.readable = 0;
	}
	commlock_unlock(&commdata->sendlock);

	if (likely(flag)) {
		int size = 0;
		size = mfptp_check_memory(commdata->send_buff.capacity - commdata->send_buff.size, message->package.packages, message->package.frames, message->package.dsize);
		if (size > 0) {
			/* 检测到内存不够 则增加内存*/
			if (unlikely(!commcache_expend(&commdata->send_buff, size))) {
				/* 增加内存失败 */
				flag = false;
			} else {
				/* 增加内存成功 */
			}
		}
		if (likely(flag)) {
			mfptp_fill_package(&commdata->packager, message->package.frame_offset, message->package.frame_size, message->package.frames_of_package, message->package.packages);
			packsize = mfptp_package(&commdata->packager, message->content, message->config, message->socket_type);
			if (packsize > 0 && commdata->packager.ms.error == MFPTP_OK) {
				commdata->send_buff.end += packsize;
				//commdata->send_buff.size += packsize;
			//	log("package successed\n");
			} else {
				flag = false;
			//	log("package failed\n");
			}
		}
		free_commmsg(message);
	}

	return flag;
}

/* 填充message结构体 */ 
static void _fill_message_package(struct comm_message *message, const struct mfptp_package_info *package) 
{
	assert(message && package);
	int i = 0, j = 0, k = 0;
	int frames = 0;
	for (i = 0; i < package->packages; i++) {
		for (j = 0; j < package->frame[i].frames; j++) {
			message->package.frame_size[k] = package->frame[i].frame_size[j];
			message->package.frame_offset[k] = package->frame[i].frame_offset[j];
			k++;
		}
		message->package.frames_of_package[i] = package->frame[i].frames;
		frames += package->frame[i].frames;
	}
	message->package.packages = package->packages;
	message->package.frames = frames;
	message->package.dsize = package->dsize;
}
