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
		int		    fd = 0;
		struct connfd_info* connfd = NULL;
		/* 删除所有类型为COMM_CONNECT和COMM_ACCEPT的fd相关信息 */
		while (commevent->connfdcnt) {
			if ((connfd = commevent->connfd[fd])) {
				commdata_destroy(connfd);
				commevent->bindfdcnt--;
				commepoll_del(&commevent->commctx->commepoll, fd, -1);
			}
			fd++;
		}
		/* 删除所有类型为COMM_BIND的fd相关信息 */
		while (commevent->bindfdcnt) {
			fd = commevent->bindfd[commevent->bindfdcnt-1].commtcp.fd;
			commepoll_del(&commevent->commctx->commepoll, fd, -1);
			close(fd);
			commevent->bindfd[commevent->bindfdcnt-1].commtcp.fd = -1;
			commevent->bindfdcnt -= 1;
		}
		free_remainfd(&commevent->remainfd);
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
	//struct connfd_info* connfd = NULL;

	while (1) {	/* 循环处理accept直到所有新连接都处理完毕退出 */
		fd = socket_accept(&commevent->bindfd[fdidx].commtcp, &commtcp);
		if (fd > 0) {
			if (commevent_add(commevent, &commtcp, &commevent->bindfd[fdidx].finishedcb)) {
				if (commevent->bindfd[fdidx].finishedcb.callback) {
					commevent->bindfd[fdidx].finishedcb.callback(commevent->commctx, &commtcp, commevent->bindfd[fdidx].finishedcb.usr);
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

	struct connfd_info *connfd = NULL;

	if ((connfd = commevent->connfd[fd])) {
		if (_read_data(commevent, connfd)) {
			del_remainfd(&commevent->remainfd, fd, REMAINFD_READ);
			if (commevent->connfd[fd]) {
				add_remainfd(&commevent->remainfd, fd, REMAINFD_PARSE);
				//commdata_parse(connfd, commevent);
			}
		}
	} else {
		del_remainfd(&commevent->remainfd, fd, REMAINFD_READ);
	}
	//log("commevent_recv rcnt:%d wcnt:%d packcnt:%d parscnt:%d\n",commevent->remainfd.rcnt, commevent->remainfd.wcnt, commevent->remainfd.packcnt, commevent->remainfd.parscnt);
	return ;
}

/* 发送数据 */
void  commevent_send(struct comm_event *commevent, int fd, bool remainfd)
{
	assert(commevent && commevent->init && fd > 0);

	struct connfd_info *connfd = NULL;

	if ((connfd = commevent->connfd[fd])) {
		/* 先进行打包数据 打包成功再发送数据 */
		//commdata_package(connfd, commevent);
		if (_write_data(commevent, connfd)) {
			del_remainfd(&commevent->remainfd, fd, REMAINFD_WRITE);
		}
	} else {
		del_remainfd(&commevent->remainfd, fd, REMAINFD_WRITE);
	}
	//log("commevent_send rcnt:%d wcnt:%d packcnt:%d parscnt:%d\n",commevent->remainfd.rcnt, commevent->remainfd.wcnt, commevent->remainfd.packcnt, commevent->remainfd.parscnt);
	return ;
}

void commevent_remainfd(struct comm_event *commevent, bool timeout)
{
	assert(commevent && commevent->init);
	int fd = -1;
	int counter = 0;	/* 处理fd个数的计数 */
	/* 先处理需要读取数据的fd */
	while (counter < MAX_DISPOSE_FDS && (commevent->remainfd.cnt[0] || commevent->remainfd.cnt[1] || commevent->remainfd.cnt[2] || commevent->remainfd.cnt[3] || commevent->remainfd.cnt[4])) {
		if (commevent->remainfd.cnt[0]) {
			/* 处理Accept事件 */
			/* 类型为REMAINFD_LISTEN类型的fd里面保存的是fd的索引 */
			int fdidx = commevent->remainfd.fda[0][commevent->remainfd.circle[0]];
			log("commevent_remainfd accept fd:%d, index:%d,cnt:%d\n", fd, commevent->remainfd.circle[0], commevent->remainfd.cnt[0]);
			//fdidx = gain_bindfd_fdidx(commevent, fd);
			commevent_accept(commevent, fdidx);
			del_remainfd(&commevent->remainfd, fdidx, REMAINFD_LISTEN);
			if (commevent->remainfd.cnt[0] > 0) {
				commevent->remainfd.circle[0] = (commevent->remainfd.circle[0] + 1)%commevent->remainfd.cnt[0];
			} else {
				commevent->remainfd.circle[0] = 0;
			}
		}
		if (commevent->remainfd.cnt[1] > 0) {
			/* 处理读事件 */
			fd = commevent->remainfd.fda[1][commevent->remainfd.circle[1]];
			log("commevent_remainfd read fd:%d, index:%d,cnt:%d\n", fd, commevent->remainfd.circle[1], commevent->remainfd.cnt[1]);
			commevent_recv(commevent, fd, true);
			if (commevent->remainfd.cnt[1] > 0) {
				commevent->remainfd.circle[1] = (commevent->remainfd.circle[1] + 1)%commevent->remainfd.cnt[1];
			} else {
				commevent->remainfd.circle[1] = 0;
			}
		}

		if (commevent->remainfd.cnt[2] > 0) {
			/* 处理解析事件 */
			fd = commevent->remainfd.fda[2][commevent->remainfd.circle[2]];
			log("commevent_remainfd parse fd:%d, index:%d,cnt:%d\n", fd, commevent->remainfd.circle[2], commevent->remainfd.cnt[2]);
			if (commevent->connfd[fd]) {
				commdata_parse(commevent->connfd[fd], commevent);
			} else {
				del_remainfd(&commevent->remainfd, fd, REMAINFD_PARSE);
			}
			if (commevent->remainfd.cnt[2] > 0) {
				commevent->remainfd.circle[2] = (commevent->remainfd.circle[2] + 1)%commevent->remainfd.cnt[2];
			} else {
				commevent->remainfd.circle[2] = 0;
			}
		}

		if (commevent->remainfd.cnt[3] > 0) {
			/* 处理打包事件 */
			fd = commevent->remainfd.fda[3][commevent->remainfd.circle[3]];
			log("commevent_remainfd package fd:%d, index:%d,cnt:%d\n", fd, commevent->remainfd.circle[3], commevent->remainfd.cnt[3]);
			if (commevent->connfd[fd]) {
				if (commdata_package(commevent->connfd[fd], commevent)) {
					add_remainfd(&commevent->remainfd, fd, REMAINFD_WRITE);
				}
			//	commevent_send(commevent, fd, true);
			} else {
				del_remainfd(&commevent->remainfd, fd, REMAINFD_PACKAGE);
			}
			if (commevent->remainfd.cnt[3] > 0) {
				commevent->remainfd.circle[3] = (commevent->remainfd.circle[3] + 1)%commevent->remainfd.cnt[3];
			} else {
				commevent->remainfd.circle[3] = 0;
			}
		}

		if (commevent->remainfd.cnt[4] > 0) {
			/* 处理写事件 */
			fd = commevent->remainfd.fda[4][commevent->remainfd.circle[4]];
			log("commevent_remainfd write fd:%d, index:%d,cnt:%d\n", fd, commevent->remainfd.circle[4], commevent->remainfd.cnt[4]);
			commevent_send(commevent, fd, true);
			if (commevent->remainfd.cnt[4] > 0) {
				commevent->remainfd.circle[4] = (commevent->remainfd.circle[4] + 1)%commevent->remainfd.cnt[4];
			} else {
				commevent->remainfd.circle[4] = 0;
			}
		}
		counter ++;
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
		struct connfd_info* connfd = NULL;
		if (commepoll_add(&commevent->commctx->commepoll, commtcp->fd, EPOLLIN | EPOLLOUT | EPOLLET)) {
			if (commdata_init(&connfd, commtcp, finishedcb)) {
				commevent->connfd[commtcp->fd] = connfd;
				commevent->connfdcnt++;
				return true;
			}
		}
		
	} else {
		if (commepoll_add(&commevent->commctx->commepoll, commtcp->fd, EPOLLIN | EPOLLET)) {
			memcpy(&commevent->bindfd[commevent->bindfdcnt].commtcp, commtcp, sizeof(*commtcp));
			if (finishedcb) {
				memcpy(&commevent->bindfd[commevent->bindfdcnt].finishedcb, finishedcb, sizeof(*finishedcb));
			}
			commevent->bindfdcnt ++;
			return true;
		}
	}
	return false;
}

void commevent_del(struct comm_event *commevent, int fd)
{
	assert(commevent && commevent->init && fd > 0);
	int			fdidx = -1;
	struct connfd_info*	connfd = NULL;
	commepoll_del(&commevent->commctx->commepoll, fd, -1);
	if ((connfd = commevent->connfd[fd])) {
		commdata_destroy(connfd);
		commevent->connfd[fd] = 0;
		commevent->connfdcnt --;
	} else if ((fdidx = gain_bindfd_fdidx(commevent, fd)) > -1) {
		if ((fdidx + 1) != commevent->bindfdcnt) {
			/* 如果删除的fd不是最后一个fd，则将后面的fd数据往前拷贝 */
			int size = (sizeof(struct comm_tcp)) * (commevent->bindfdcnt - fdidx -2);
			memmove(&commevent->bindfd[fdidx].commtcp, &commevent->bindfd[fdidx+1].commtcp, size);
			memmove(&commevent->bindfd[fdidx].finishedcb, &commevent->bindfd[fdidx+1].finishedcb, size);
		}
		commevent->bindfdcnt --;
	}
}

/* @返回值：false 代表此描述符没有处理完毕，需要保存起来下次继续处理 true 代表此描述符已处理完毕 */
static  bool _write_data(struct comm_event *commevent, struct connfd_info *connfd)
{
	assert(commevent && commevent->init && connfd);

	int  bytes = 0;
	bool flag = true;

	connfd->commtcp.stat = FD_WRITE;
	if (check_writeable(connfd->commtcp.fd)) {
		if (connfd->send_cache.size > 0) {
			bytes = write(connfd->commtcp.fd, &connfd->send_cache.buffer[connfd->send_cache.start], connfd->send_cache.size);
			if (bytes > 0) {
				if (unlikely(bytes < connfd->send_cache.size)) {
					/* 数据没有一次性发送完毕，说明系统缓冲区已满，返回false，下次继续发送剩下的数据 */
					flag = false;
				} else {
				//	log("write data successed fd:%d\n", connfd->commtcp.fd);
				}
				connfd->send_cache.start += bytes;
				connfd->send_cache.size -= bytes;
				commcache_clean(&connfd->send_cache);
			} else {
				if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK ) {
					flag = false;
				} else {
					/* 出现其他的致命错误，返回true，将此fd的相关信息删除 */
					DELETEFD(commevent, connfd->commtcp.fd);
					return true;
				}
			}
			if (flag && connfd->finishedcb.callback) {
				connfd->finishedcb.callback(commevent->commctx, &connfd->commtcp, connfd->finishedcb.usr);
			}
		}
		return flag;
	}

	DELETEFD(commevent, connfd->commtcp.fd);	/* 描述符不可写，则将此描述符的相关信息全部删除 */
	return flag;
}

/* @返回值：false 代表此描述符没有处理完毕，需要保存起来下次继续处理 true 代表此描述符已处理完毕 */
static  bool _read_data(struct comm_event *commevent, struct connfd_info *connfd)
{
	assert(commevent && commevent->init && connfd);

	int  bytes = 0;
	bool flag = true;
	char buff[COMM_READ_MIOU] = {};

	connfd->commtcp.stat = FD_READ;
	if (check_readable(connfd->commtcp.fd)) {
		while((bytes = read(connfd->commtcp.fd, buff, COMM_READ_MIOU)) > 0) {
			commcache_append(&connfd->recv_cache, buff, bytes);
			if (bytes < COMM_READ_MIOU) {		/* 数据已经读取完毕 */
				break ;
			}
		}
		if (bytes < 0) {
			if (errno == EINTR) {					/* fd被打断，返回false，留到下一次进行处理 */
				flag = false;
			} else if (errno != EAGAIN || errno != EWOULDBLOCK){	/* 发生致命错误，则删除fd的相关信息 */
				DELETEFD(commevent, connfd->commtcp.fd);
				return true;
			}
		} else if (bytes == 0) {					/* 对端已关闭 */
			connfd->commtcp.stat = FD_CLOSE;
		}
		if (flag && connfd->finishedcb.callback) {
			connfd->finishedcb.callback(commevent->commctx, &connfd->commtcp, connfd->finishedcb.usr);
		}
		if (connfd->commtcp.stat == FD_CLOSE) {
			DELETEFD(commevent, connfd->commtcp.fd);
			log("%d closed\n", connfd->commtcp.fd);
		} else {
		//	log("read data successed:%d\n", connfd->commtcp.fd);
		}
		return flag;
	}

	DELETEFD(commevent, connfd->commtcp.fd);	/* 此描述符不可读，则直接将此描述符相关信息删除 */
	return flag;
}
