/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_event.h"

#define		COMM_READ_MIOU	1024	/* 允许一次性读取数据的大小 */

static bool _write_data(struct comm_event *commevent, int fd);

static bool _read_data(struct comm_event *commevent, int fd);


bool commevent_init(struct comm_event** commevent, struct comm_context* commctx)
{
	assert(commevent);
	New(*commevent);
	if (*commevent) {
		(*commevent)->init = true;
		(*commevent)->commctx = commctx;
	}
	return (*commevent)->init;
}

void commevent_destroy(struct comm_event* commevent)
{
	assert(commevent && commevent->init);
	if (commevent && commevent->init) {
		struct comm_data *commdata = NULL;
		int fd = 0;
		while (commevent->fds) {
			commdata = (struct comm_data*)commevent->data[fd];
			if (commdata) {
				commdata_destroy(commdata);
				commevent->fds--;
				commepoll_del(&commevent->commctx->commepoll, fd, -1);
			}
			fd++;
		}
		while (commevent->listenfd.counter) {
			fd = commevent->listenfd.commtcp[commevent->listenfd.counter].fd;
			commepoll_del(&commevent->commctx->commepoll, fd, -1);
			commevent->listenfd.counter --;
		}
		commevent->init = false;
		Free(commevent);
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
	struct comm_data*	commdata = (struct comm_data*)commevent->data[fd];
	commepoll_del(&commevent->commctx->commepoll, fd, -1);
	if (commdata) {
		commdata_destroy(commdata);
		commevent->data[fd] = 0;
		commevent->fds --;
	} else if ((fdidx = gain_listenfd_fdidx(&commevent->listenfd, fd)) > -1) {
		//memset(&commevent->listenfd.commtcp[fdidx], 0, sizeof(commevent->listenfd.commtcp[fdidx]));
		//memset(&commevent->listenfd.finishedcb[fdidx], 0, sizeof(commevent->listenfd.finishedcb[fdidx]));
		commevent->listenfd.counter --;
	}
}

void commevent_timeout(struct comm_event *commevent)
{
	assert(commevent && commevent->init);
	int fd = 0;
	int counter = commevent->fds;
	while (counter) {
		struct comm_data *commdata = (struct comm_data*)commevent->data[fd];
		if (likely(commdata)) {
			//接收缓冲区里面存在数据需要解析
			if (likely(commdata->recv_cache.size > 0)) {

				commdata_parse(commdata, commevent);
			}
			//发送缓冲区里面存在数据需要打包
			if (likely(commdata->send_cache.size > 0)) {
				commdata_package(commdata, commevent);
			}
			counter--;
		}
		fd++;
	}

	if (likely(commevent->timeoutcb.callback)) { //调用用户层的回调函数
		commevent->timeoutcb.callback(commevent->commctx, NULL, commevent->timeoutcb.usr);
	}
}

void  commevent_accept(struct comm_event* commevent, int fdidx)
{
	assert(commevent && commevent->init);

	int fd = -1;
	struct comm_tcp		commtcp = {};
	struct comm_data*	commdata = NULL;

	/* 循环处理accept直到所有新连接都处理完毕退出 */
	while (1) {

		fd = socket_accept(&commevent->listenfd.commtcp[fdidx], &commtcp);
		if (fd > 0) {
			if (commevent_add(commevent, &commtcp, &commevent->listenfd.finishedcb[fdidx])) {
				if (commevent->listenfd.finishedcb[fdidx].callback) {
					commevent->listenfd.finishedcb[fdidx].callback(commevent->commctx, &commtcp, commevent->listenfd.finishedcb[fdidx].usr);
				}
			} else {
				close(fd);
			}
		} else if (fd == -1){
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
void commevent_recv(struct comm_event *commevent, int fd)
{
	assert(commevent && commevent->init);

	int n = 0;
	int bytes = 0;

	/* 先处理残留下来没处理完的fd */
	for (n = 0; n < commevent->remainfd.rcnt; n++) {
		if (_read_data(commevent, commevent->remainfd.rfda[n])) {
			/* 接收数据成功 */
			commevent->remainfd.rcnt -= 1;
		} else {
			/* 接收数据失败*/
			commevent->remainfd.rfda[commevent->remainfd.rcnt-1] = commevent->remainfd.rfda[n];
			commevent->remainfd.rcnt += 1;
		}
	}
	if (_read_data(commevent, fd)) {
		/* 读取数据成功就进行解析 */
		struct comm_data *commdata = (struct comm_data *)commevent->data[fd];
		commdata_parse(commdata, commevent);
		commcache_restore(&commdata->recv_cache);
	} else {
		/* 读取数据失败 */
		commevent->remainfd.rfda[commevent->remainfd.rcnt-1] = commevent->remainfd.rfda[n];
		commevent->remainfd.rcnt += 1;
	}

	return ;
}

/* 发送数据 */
void  commevent_send(struct comm_event *commevent, int* fda, int cnt)
{
	assert(commevent && commevent->init && fda);
	int n = 0;
	int bytes = 0;

	/* 先处理遗留的没有处理的fd */
	for (n = 0; n < commevent->remainfd.wcnt; n++) {
		if (likely(_write_data(commevent, commevent->remainfd.wfda[n]))) {
			/* 数据发送成功 */
			commevent->remainfd.wcnt -= 1;
		} else {
			/* 数据发送失败 将此未处理的fd保存起来 下次进行处理 */
			commevent->remainfd.wfda[commevent->remainfd.wcnt-1] = commevent->remainfd.wfda[n];
			commevent->remainfd.wcnt += 1;
		}
	}

	/* 开始处理新的fd */
	for (n = 0; n < cnt; n++) {
		/* 处理新fd时先进行打包，然后在进行数据发送,如果打包失败则会直接将此包丢掉，不作处理 */
		if (commdata_package((struct comm_data*)commevent->data[fda[n]], commevent)) {
			if (!_write_data(commevent, fda[n])) {
				/* 数据发送失败 将此未处理的fd保存起来 下次进行处理 */
				commevent->remainfd.wfda[commevent->remainfd.wcnt-1] = fda[n];
				commevent->remainfd.wcnt += 1;
			}
		}
	}
	return ;
}

static  bool _write_data(struct comm_event *commevent, int fd)
{
	assert(commevent && commevent->init && fd > 0);

	bool			flag = false;
	struct comm_data*	commdata = NULL;

	if ((commdata = (struct comm_data*)commevent->data[fd])) {
		if (commdata->commtcp.stat != FD_INIT) {
			if (check_writeable(fd)) {
				if (commdata->send_cache.size > 0) {
					int bytes = 0;
					bytes = write(fd, &commdata->send_cache.buffer[commdata->send_cache.start], commdata->send_cache.size);
					if (bytes > 0) {
						commdata->send_cache.start += bytes;
						commdata->send_cache.size -= bytes;
						commcache_clean(&commdata->send_cache);
						if (commdata->finishedcb.callback) {
							commdata->finishedcb.callback(commevent->commctx, &commdata->commtcp, commdata->finishedcb.usr);
						}
						log("write data successed fd:%d\n", fd);
						flag = true;
					}
				}
			} else {
				/* 描述符不可写，则将此描述符的相关信息全部删除 */
				commevent_del(commevent, fd);
				close(fd);
			}
		} else {
			/* 第一次强制触发的写事件，直接正确返回 */
			flag = true;
		}
		commdata->commtcp.stat = FD_WRITE;
	} else {
		flag = true;
	}
	return flag;
}

static  bool _read_data(struct comm_event *commevent, int fd)
{
	assert(commevent && commevent->init && fd > 0);
	int bytes = 0;
	bool flag = false;
	char buff[COMM_READ_MIOU] = {};
	struct comm_data *commdata = NULL;

	if ((commdata = (struct comm_data*)commevent->data[fd])) {

		while (1) {		/* 当此描述符的全部数据都读取完毕才退出 */
			bytes = read(fd, buff, COMM_READ_MIOU);
			if (likely(bytes > 0)) {
				commcache_append(&commdata->recv_cache, buff, bytes);
				if (bytes < COMM_READ_MIOU) {		/* 数据已经读取完毕 */
					//log("read data successed\n");
					flag = true;
					break ;
				}
			} else if (bytes == 0) {			/* 对端已经关闭 */
				flag = true;
				commdata->commtcp.stat = FD_CLOSE;
				break ;
			} else {
				if (errno == EAGAIN || errno == EWOULDBLOCK) { /* 数据已经读取完毕 */
					//log("read data successed\n");
					flag = true;
					break ;
				} else {
					//log("read data failed\n");
					flag = false;
				}
			}
		}
		commdata->commtcp.stat = FD_READ;
		if (flag && commdata->finishedcb.callback) {
			commdata->finishedcb.callback(commevent->commctx, &commdata->commtcp, commdata->finishedcb.usr);
		}
		if (commdata->commtcp.stat == FD_CLOSE) {
			commevent_del(commevent, fd);
			close(fd);
			log("close fd :%d\n", fd);
		}
	} else {
		flag = true;
	}
	return flag;
}
