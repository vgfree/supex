/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_event.h"


/* 允许从socket fd 一次性读取数据的大小 */
#define	COMM_READ_MIOU	1024	

/* 处理残留fd时，允许连续处理同一事件[读或写]fd的个数最大个数 */
#define	 MAX_DISPOSE_FDS  5

/* 删除一个fd的相关信息以及关闭一个fd */
#define	DELETEFD(commevent, fd)	({	commevent_del(commevent, fd); \
					close(fd);		\
				})

static bool _write_data(struct comm_event *commevent, struct comm_data *commdata);

static bool _read_data(struct comm_event *commevent, struct comm_data *commdata);

bool commevent_init(struct comm_event **commevent, struct comm_context *commctx)
{
	assert(commevent && commctx);

	New(*commevent);
	if (*commevent) {
		(*commevent)->init = true;
		(*commevent)->commctx = commctx;
	}
	return (*commevent)->init;
}

void commevent_destroy(struct comm_event *commevent)
{
	if (commevent && commevent->init) {
		int		  fd = 0;
		struct comm_data* commdata = NULL;
		/* 删除所有类型为COMM_CONNECT和COMM_ACCEPT的fd相关信息 */
		while (commevent->fds) {
			if ((commdata = (struct comm_data*)commevent->data[fd])) {
				commdata_destroy(commdata);
				commevent->fds--;
				commepoll_del(&commevent->commctx->commepoll, fd, -1);
			}
			fd++;
		}
		/* 删除所有类型为COMM_BIND的fd相关信息 */
		while (commevent->listenfd.counter) {
			fd = commevent->listenfd.commtcp[commevent->listenfd.counter].fd;
			commevent->listenfd.counter --;
			commepoll_del(&commevent->commctx->commepoll, fd, -1);
		}
		commevent->init = false;
		Free(commevent);
	}
	return ;
}

/* 有新客户端连接上服务器 */
void  commevent_accept(struct comm_event *commevent, int fdidx)
{
	assert(commevent && commevent->init && fdidx > -1);

	int		  fd = -1;
	struct comm_tcp	  commtcp = {};
	struct comm_data* commdata = NULL;

	while (1) {	/* 循环处理accept直到所有新连接都处理完毕退出 */
		fd = socket_accept(&commevent->listenfd.commtcp[fdidx], &commtcp);
		if (fd > 0) {
			if (commevent_add(commevent, &commtcp, &commevent->listenfd.finishedcb[fdidx])) {
				if (commevent->listenfd.finishedcb[fdidx].callback) {
					commevent->listenfd.finishedcb[fdidx].callback(commevent->commctx, &commtcp, commevent->listenfd.finishedcb[fdidx].usr);
				}
			} else {	/* fd添加到struct comm_event中进行监控失败，则忽略并关闭此描述符 */
				close(fd);
			}
		} else if (fd == -1) {
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

/* 接收数据 */
void commevent_recv(struct comm_event *commevent, int fd, bool remainfd)
{
	assert(commevent && commevent->init && fd > 0);

	struct comm_data *commdata = NULL;

	if ((commdata = (struct comm_data*)commevent->data[fd])) {
		if (_read_data(commevent, commdata)) {
			/* 返回真，判断commevent->data[fd]不为0说明读取数据成功 再进行解析 */
			if (commevent->data[fd] && commdata_parse(commdata, commevent, false)) {
				/* 解析成功之后恢复cache的缓冲区 */
				commcache_restore(&commdata->recv_cache);
			}
			if (remainfd) {
				commevent->remainfd.rpcnt -= 1;
			}
		} else {
			/* 读取数据失败 */
			commevent->remainfd.rpfds.fda[commevent->remainfd.rpcnt] = fd;
			commevent->remainfd.rpfds.type[commevent->remainfd.rpcnt] = REMAIN_READ;
			commevent->remainfd.rpcnt += 1;
		}
	}
	return ;
}

/* 发送数据 */
void  commevent_send(struct comm_event *commevent, int fd, bool remainfd)
{
	assert(commevent && commevent->init && fd > 0);

	struct comm_data *commdata = NULL;

	if ((commdata = (struct comm_data*)commevent->data[fd])) {
		/* 先进行打包数据 打包成功再发送数据 */
		if (commdata_package(commdata, commevent)) {
			if (commdata->commtcp.stat == FD_WRITE) {
				if (!_write_data(commevent, commdata)) {
					/* 数据发送失败 将此fd保存起来 下次进行处理 */
					commevent->remainfd.wpfda[commevent->remainfd.wpcnt] = fd;
					commevent->remainfd.wpcnt += 1;
				} else if (remainfd) {
					/* 处理的是残留的fd并且成功处理完毕 */
					commevent->remainfd.wpcnt -= 1;
				}
			} else {
				/* 强制性触发的第一次，将FD_INIT状态改为FD_WRITE*/
				commdata->commtcp.stat = FD_WRITE;
			}
		}
	}
	return ;
}

void commevent_remainfd(struct comm_event *commevent, bool timeout)
{
	assert(commevent && commevent->init);
	int fd = -1;
	int counter = 0;	/* 处理fd个数的计数 */
	/* 先处理需要读取数据的fd */
	while (counter < MAX_DISPOSE_FDS && counter < commevent->remainfd.rpcnt) {
		fd = commevent->remainfd.rpfds.fda[commevent->remainfd.rpcnt-1];
		if (commevent->remainfd.rpfds.type[commevent->remainfd.rpcnt-1] == REMAIN_PARSE) {
			struct comm_data *commdata = (struct comm_data*)commevent->data[fd];
			if (commdata) {
				commdata_parse(commdata, commevent, true);
			} else {
				commevent->remainfd.rpcnt -= 1;
			}
		} else {
			commevent_recv(commevent, fd, true);
		}
		counter ++;
		log("here in timeout event dispose readable fds\n");
	}
	counter = 0;

	/* 再处理需要发送数据的fd */
	while (counter < MAX_DISPOSE_FDS && counter < commevent->remainfd.wpcnt) {
		commevent_send(commevent, commevent->remainfd.wpfda[commevent->remainfd.wpcnt-1], true);
		counter ++;
		log("here in timeout event dispose writeable fds\n");
	}

	/* 超时调用该函数则调用用户层的回调函数 */
	if (timeout && commevent->timeoutcb.callback) { 
		commevent->timeoutcb.callback(commevent->commctx, NULL, commevent->timeoutcb.usr);
	}
}

bool commevent_add(struct comm_event *commevent, struct comm_tcp *commtcp, struct cbinfo *finishedcb)
{
	assert(commevent && commevent->init && commtcp && commtcp->fd > 0);

	if (commtcp->type == COMM_CONNECT || commtcp->type == COMM_ACCEPT) {
		struct comm_data *commdata = NULL;
		if (commepoll_add(&commevent->commctx->commepoll, commtcp->fd, EPOLLIN | EPOLLOUT | EPOLLET)) {
			if (commdata_init(&commdata, commtcp, finishedcb)) {
				commevent->data[commtcp->fd] = (intptr_t)commdata;
				commevent->fds ++;
				return true;
			}
		}
		
	} else {
		if (commepoll_add(&commevent->commctx->commepoll, commtcp->fd, EPOLLIN | EPOLLET)) {
			memcpy(&commevent->listenfd.commtcp[commevent->listenfd.counter], commtcp, sizeof(*commtcp));
			if (finishedcb) {
				memcpy(&commevent->listenfd.finishedcb[commevent->listenfd.counter], finishedcb, sizeof(*finishedcb));
			}
			commevent->listenfd.counter ++;
			return true;
		}
	}
	return false;
}

void commevent_del(struct comm_event *commevent, int fd)
{
	assert(commevent && commevent->init && fd > 0);
	int			fdidx = -1;
	struct comm_data*	commdata = NULL;
	commepoll_del(&commevent->commctx->commepoll, fd, -1);
	if ((commdata = (struct comm_data*)commevent->data[fd])) {
		commdata_destroy(commdata);
		commevent->data[fd] = 0;
		commevent->fds --;
	} else if ((fdidx = gain_listenfd_fdidx(&commevent->listenfd, fd)) > -1) {
		if ((fdidx + 1) != commevent->listenfd.counter) {
			/* 如果删除的fd不是最后一个fd，则将后面的fd数据往前拷贝 */
			struct listenfd* listenfd = &commevent->listenfd;
			memmove(&listenfd->commtcp[fdidx], &listenfd->commtcp[fdidx+1], (sizeof(struct comm_tcp)*(listenfd->counter - fdidx -2)));
			memmove(&listenfd->finishedcb[fdidx], &listenfd->finishedcb[fdidx+1], (sizeof(struct cbinfo)*(listenfd->counter - fdidx -2)));
		}
		commevent->listenfd.counter --;
	}
}

/* @返回值：false 代表此描述符没有处理完毕，需要保存起来下次继续处理 true 代表此描述符已处理完毕 */
static  bool _write_data(struct comm_event *commevent, struct comm_data *commdata)
{
	assert(commevent && commevent->init && commdata);

	int  bytes = 0;
	bool flag = true;

	commdata->commtcp.stat = FD_WRITE;
	if (check_writeable(commdata->commtcp.fd)) {
		if (commdata->send_cache.size > 0) {
			bytes = write(commdata->commtcp.fd, &commdata->send_cache.buffer[commdata->send_cache.start], commdata->send_cache.size);
			if (bytes > 0) {
				if (bytes < commdata->send_cache.size) {
					/* 数据没有一次性发送完毕，说明系统缓冲区已满，返回false，下次继续发送剩下的数据 */
					flag = false;
				} else {
					log("write data successed fd:%d\n", commdata->commtcp.fd);
				}
				commdata->send_cache.start += bytes;
				commdata->send_cache.size -= bytes;
				commcache_clean(&commdata->send_cache);
			} else {
				if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK ) {
					flag = false;
				} else {
					/* 出现其他的致命错误，返回true，将此fd的相关信息删除 */
					DELETEFD(commevent, commdata->commtcp.fd);
					return true;
				}
			}
			if (flag && commdata->finishedcb.callback) {
				commdata->finishedcb.callback(commevent->commctx, &commdata->commtcp, commdata->finishedcb.usr);
			}
		}
		return flag;
	}

	DELETEFD(commevent, commdata->commtcp.fd);	/* 描述符不可写，则将此描述符的相关信息全部删除 */
	return flag;
}

/* @返回值：false 代表此描述符没有处理完毕，需要保存起来下次继续处理 true 代表此描述符已处理完毕 */
static  bool _read_data(struct comm_event *commevent, struct comm_data *commdata)
{
	assert(commevent && commevent->init && commdata);

	int  bytes = 0;
	bool flag = true;
	char buff[COMM_READ_MIOU] = {};

	commdata->commtcp.stat = FD_READ;
	if (check_readable(commdata->commtcp.fd)) {
		while((bytes = read(commdata->commtcp.fd, buff, COMM_READ_MIOU)) > 0) {
			commcache_append(&commdata->recv_cache, buff, bytes);
			if (bytes < COMM_READ_MIOU) {		/* 数据已经读取完毕 */
				break ;
			}
		}
		if (bytes < 0) {
			if (errno == EINTR) {					/* fd被打断，返回false，留到下一次进行处理 */
				flag = false;
			} else if (errno != EAGAIN || errno != EWOULDBLOCK){	/* 发生致命错误，则删除fd的相关信息 */
				DELETEFD(commevent, commdata->commtcp.fd);
				return true;
			}
		} else if (bytes == 0) {					/* 对端已关闭 */
			commdata->commtcp.stat = FD_CLOSE;
		}
		if (flag && commdata->finishedcb.callback) {
			commdata->finishedcb.callback(commevent->commctx, &commdata->commtcp, commdata->finishedcb.usr);
		}
		if (commdata->commtcp.stat == FD_CLOSE) {
			DELETEFD(commevent, commdata->commtcp.fd);
			log("%d closed\n", commdata->commtcp.fd);
		} else {
			log("read data successed:%d\n", commdata->commtcp.fd);
		}
		return flag;
	}

	DELETEFD(commevent, commdata->commtcp.fd);	/* 此描述符不可读，则直接将此描述符相关信息删除 */
	return flag;
}
