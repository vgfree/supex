/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_event.h"

static bool _write_data(struct comm_data *commdata, int fd);

static bool _read_data(struct comm_data *commdata, int fd);

void timeout_event(struct comm_context* commctx)
{
	int fd = 0;
	int counter = commctx->commepoll.watchcnt - commctx->listenfd.counter -1;
	while (counter) {
		struct comm_data *commdata = (struct comm_data*)commctx->data[fd];
		if (likely(commdata)) {
			//接收缓冲区里面存在数据需要解析
			if (likely(commdata->recv_buff.end - commdata->recv_buff.start > 0)) {

				parse_data(commdata);
			}
			//发送缓冲区里面存在数据需要打包
			if (likely(commdata->send_buff.end - commdata->recv_buff.start > 0)) {
				package_data(commdata);
			}
			counter--;
		}
		fd++;
	}
	if (likely(commctx->timeoutcb.callback)) { //调用用户层的回调函数
		commctx->timeoutcb.callback(commctx, NULL, commctx->timeoutcb.usr);
	}
}

void  accept_event(struct comm_context *commctx, int fdidx)
{
	assert(commctx);
	int fd = -1;

	/* 循环处理accept直到所有新连接都处理完毕退出 */
	while (1) {

		struct portinfo		portinfo = {};
		struct comm_data*	commdata = NULL;
		fd = accept(commctx->listenfd.fd[fdidx], NULL, NULL);
		if (likely(fd > 0)) {
			if (unlikely(!fd_setopt(fd, O_NONBLOCK))) {
				close(fd);
				continue ;
			}
				
			if (unlikely(!get_portinfo(&portinfo, fd, COMM_ACCEPT, FD_INIT))) {
				close(fd);
				continue ;
			}
			commdata = commdata_init(commctx, &portinfo, &commctx->listenfd.finishedcb[fdidx]);
			if (likely(commdata)) {
				commctx->data[fd ] = (intptr_t)commdata;
				if (commctx->listenfd.finishedcb[fdidx].callback) {
					commctx->listenfd.finishedcb[fdidx].callback(commctx, &portinfo, commctx->listenfd.finishedcb[fdidx].usr);
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


void recv_event(struct comm_context *commctx, int fd)
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
			/* 数据发送成功 */
		} else {
			/* 数据发送失败 */
			commctx->remainfd.rfda[commctx->remainfd.rcnt-1] = commctx->remainfd.rfda[n];
			commctx->remainfd.rcnt += 1;
		}
	}
	return ;
}

void  send_event(struct comm_context *commctx, int* fda, int cnt)
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
	commdata->portinfo.stat = FD_WRITE;

	int bytes = 0;
	bool flag = false;

	if (likely(check_writeable(fd))) {
		log("check fd writeable\n");
	} else {
		log("check fd not writeable\n");
		/* fd已经不可写，则需要将此fd相关的数据全部删除 */
		return false;
	}
	do {
		if (likely(commdata->send_buff.size > 0)) {
			bytes = write(fd, &commdata->send_buff.cache[commdata->send_buff.start], commdata->send_buff.size);
			if (bytes < 0) {
				//log("write data failed fd:%d\n", fd);
				return false;
			} else {
				commdata->send_buff.start += bytes;
				commdata->send_buff.size -= bytes;
				commcache_clean(&commdata->send_buff);
				log("write data successed fd:%d\n", fd);
				if (commdata->finishedcb.callback) {
					commdata->finishedcb.callback(commdata->commctx, &commdata->portinfo, commdata->finishedcb.usr);
				}
				return true;
			}
		} else {
			/* 暂时没数据发送的时候就去打包一次数据 */
			flag = 	package_data(commdata); 
		}
		
	} while(flag);/* 打包成功再去发送一次数据，打包失败，则直接退出循环*/

	return false;
}

static  bool _read_data(struct comm_data *commdata, int fd)
{
	assert(commdata);
	int bytes = 0;
	bool flag = false;
	commdata->portinfo.stat = FD_READ;
	do {
		bytes = read(fd, &commdata->recv_buff.cache[commdata->recv_buff.end], COMM_READ_MIOU);
		if (likely(bytes > 0)) {
			commdata->recv_buff.size += bytes;
			commdata->recv_buff.end += bytes;
			if (bytes < COMM_READ_MIOU) {		/* 数据已经读取完毕 */
				//log("read data successed\n");
				flag = true;
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
				flag = true;
				break ;
			} else {
				//log("read data failed\n");
				return false;
			}
		}
	} while(1); /* 只有当数据全部读取完毕才退出循环 */

	if (commdata->finishedcb.callback) {
		commdata->finishedcb.callback(commdata->commctx, &commdata->portinfo, commdata->finishedcb.usr);
	}

	if (commdata->portinfo.stat == FD_CLOSE) {
		log("close fd :%d\n", fd);
		comm_close(commdata->commctx, fd);
		log("after close:%d\n", (int)commdata->commctx->data[fd]);
	}
	/* 目前暂定只要接收到数据就解析一次 */
	if (flag) {
		flag = parse_data(commdata);
	}

	return true;
}
