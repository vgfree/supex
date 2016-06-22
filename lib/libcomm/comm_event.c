/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_event.h"

/* 处理残留fd时，允许连续处理同一事件[读或写]fd的个数最大个数 */
#define  MAX_DISPOSE_FDS 5

static bool _write_data(struct comm_event *commevent, struct connfd_info *connfd);

static bool _read_data(struct comm_event *commevent, struct connfd_info *connfd);

bool commevent_init(struct comm_event **commevent, struct comm_context *commctx)
{
	assert(commevent && commctx);

	New(*commevent);

	if (*commevent) {
		if (unlikely(!init_remainfd(&((*commevent)->remainfd)))) {
			return false;
		}

		(*commevent)->init = true;
		(*commevent)->commctx = commctx;
	}

	return (*commevent)->init;
}

void commevent_destroy(struct comm_event *commevent)
{
	if (commevent && commevent->init) {
		int                     fd = 0;
		struct connfd_info      *connfd = NULL;

		/* 删除所有类型为COMM_CONNECT和COMM_ACCEPT的fd相关信息 */
		while (commevent->connfdcnt) {
			if ((connfd = commevent->connfd[fd])) {
				commdata_destroy(connfd);
				commevent->connfdcnt--;
				commepoll_del(&commevent->commctx->commepoll, fd, -1);
				log("close connfd:%d\n", fd);
			}

			fd++;
		}

		/* 删除所有类型为COMM_BIND的fd相关信息 */
		while (commevent->bindfdcnt) {
			fd = commevent->bindfd[commevent->bindfdcnt - 1].commtcp.fd;
			commepoll_del(&commevent->commctx->commepoll, fd, -1);
			close(fd);
			commevent->bindfd[commevent->bindfdcnt - 1].commtcp.fd = -1;
			commevent->bindfdcnt -= 1;
			log("close bindfd %d\n", fd);
		}

		free_remainfd(&commevent->remainfd);
		commevent->init = false;
		Free(commevent);
	}
}

/* 有新客户端连接上服务器 */
void commevent_accept(struct comm_event *commevent, int fdidx)
{
	assert(commevent && commevent->init && fdidx > -1);

	int             fd = -1;
	struct comm_tcp commtcp = {};
	// struct connfd_info* connfd = NULL;

	while (1) {	/* 循环处理accept直到所有新连接都处理完毕退出 */
		fd = socket_accept(&commevent->bindfd[fdidx].commtcp, &commtcp);

		if (fd > 0) {
			if (commdata_add(commevent, &commtcp, &commevent->bindfd[fdidx].finishedcb)) {
				log("listen fd:%d accept fd:%d\n", commevent->bindfd[fdidx].commtcp.fd, commtcp.fd);
				log("accept fd localport: %d local addr:%s peerport:%d peeraddr:%s\n", commtcp.localport, commtcp.localaddr, commtcp.peerport, commtcp.peeraddr);

				if (commevent->bindfd[fdidx].finishedcb.callback) {
					commevent->bindfd[fdidx].finishedcb.callback(commevent->commctx, &commtcp, commevent->bindfd[fdidx].finishedcb.usr);
				}
			} else {	/* fd添加到struct comm_event中进行监控失败，则忽略并关闭此描述符 */
				close(fd);
			}
		} else if (fd == -1) {
			if (unlikely((errno == ECONNABORTED) || (errno == EPROTO) || (errno == EINTR))) {
				/* 可能被打断，继续处理下一个连接 */
				continue;
			} else {
				/* 处理完毕或者出现致命错误，则直接返回 */
				return;
			}
		}
	}
}

void commevent_remainfd(struct comm_event *commevent, bool timeout)
{
	assert(commevent && commevent->init);

	int     fd = -1;
	int     counter = 0;	/* 处理fd个数的计数 */

	/* 先处理需要读取数据的fd */
	while (counter < MAX_DISPOSE_FDS && (commevent->remainfd.cnt[0] || commevent->remainfd.cnt[1] || commevent->remainfd.cnt[2] || commevent->remainfd.cnt[3] || commevent->remainfd.cnt[4])) {

		if (commevent->remainfd.cnt[0]) {
			/* 处理Accept事件 类型为REMAINFD_LISTEN类型的fd里面保存的是fd的索引 */
			int fdidx = commevent->remainfd.fda[0][commevent->remainfd.circle[0]];
			log("commevent_remainfd accept fd:%d, index:%d,cnt:%d\n", commevent->bindfd[fdidx].commtcp.fd, commevent->remainfd.circle[0], commevent->remainfd.cnt[0]);
			commevent_accept(commevent, fdidx);
			del_remainfd(&commevent->remainfd, fdidx, REMAINFD_LISTEN);
#if 0
			if (commevent->remainfd.cnt[0] > 0) {
				commevent->remainfd.circle[0] = (commevent->remainfd.circle[0] + 1) % commevent->remainfd.cnt[0];
			} else {
				commevent->remainfd.circle[0] = 0;
			}
#endif
		}

		if (commevent->remainfd.cnt[1] > 0) {
			/* 处理读事件 */
			fd = commevent->remainfd.fda[1][commevent->remainfd.circle[1]];

			 log("commevent_remainfd read fd:%d, index:%d,cnt:%d\n", fd, commevent->remainfd.circle[1], commevent->remainfd.cnt[1]);
			if (unlikely(!commdata_recv(commevent->connfd[fd], commevent, fd))) {
				/* 事件未处理完，则将下标增加1,下次轮询的时候执行下一个fd */
				commevent->remainfd.circle[1] = (commevent->remainfd.circle[1] + 1) % commevent->remainfd.cnt[1];
			}
		}

		if (commevent->remainfd.cnt[2] > 0) {
			/* 处理解析事件 */
			fd = commevent->remainfd.fda[2][commevent->remainfd.circle[2]];

			 log("commevent_remainfd parse fd:%d, index:%d,cnt:%d\n", fd, commevent->remainfd.circle[2], commevent->remainfd.cnt[2]);
			if (unlikely(!commdata_parse(commevent->connfd[fd], commevent, fd))) {
				commevent->remainfd.circle[2] = (commevent->remainfd.circle[2] + 1) % commevent->remainfd.cnt[2];
			}
		}

		if (commevent->remainfd.cnt[3] > 0) {
			/* 处理打包事件 */
			fd = commevent->remainfd.fda[3][commevent->remainfd.circle[3]];

			 log("commevent_remainfd package fd:%d, index:%d,cnt:%d\n", fd, commevent->remainfd.circle[3], commevent->remainfd.cnt[3]);
			if (unlikely(!commdata_package(commevent->connfd[fd], commevent, fd))) {
				commevent->remainfd.circle[3] = (commevent->remainfd.circle[3] + 1) % commevent->remainfd.cnt[3];
			}
		}

		if (commevent->remainfd.cnt[4] > 0) {
			/* 处理写事件 */
			fd = commevent->remainfd.fda[4][commevent->remainfd.circle[4]];

			log("commevent_remainfd write fd:%d, index:%d,cnt:%d\n", fd, commevent->remainfd.circle[4], commevent->remainfd.cnt[4]);
			if (unlikely(!commdata_send(commevent->connfd[fd], commevent, fd))) {
				commevent->remainfd.circle[4] = (commevent->remainfd.circle[4] + 1) % commevent->remainfd.cnt[4];
			}
		}

		counter++;
	}

	/* 超时调用该函数则调用用户层的回调函数 */
	if (timeout && commevent->timeoutcb.callback) {
		commevent->timeoutcb.callback(commevent->commctx, NULL, commevent->timeoutcb.usr);
	}
}

