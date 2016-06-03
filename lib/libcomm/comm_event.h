/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_EVENT_H__
#define __COMM_EVENT_H__

#include "comm_structure.h"
#include "comm_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 检测fd是否为COMM_BIND类型的fd并保存于struct listenfd结构体中的数组中  @返回值：-1代表不是，否则返回fd所在数组的下标 */
static inline int gain_bindfd_fdidx(struct comm_event *commevent, int fd)
{
	assert(commevent && commevent->init && fd > 0);
	int fdidx = 0;
	for (fdidx = 0; fdidx < commevent->bindfdcnt; fdidx++) {
		if (fd == commevent->bindfd[fdidx].commtcp.fd) {
			return fdidx;
		}
	}
	return -1;
}


/* 初始化事件结构体 */
bool commevent_init(struct comm_event **commevent, struct comm_context *commctx);

/* 销毁一个事件结构体 */
void commevent_destroy(struct comm_event *commevent);

/* 接收新用户的连接 @fdidex: 监听fd在结构体struct listenfd数组中的下标 */
void commevent_accept(struct comm_event *commevent, int fdidx);

/* 接收数据 @fd:此次需要读取数据的fd @remainfd:此fd是否为残留的fd */
void commevent_recv(struct comm_event *commevent, int fd, bool remainfd);

/* 发送数据 @fd:此次需要发送数据的fd @remainfd:此fd是否为残留的fd */
void commevent_send(struct comm_event *commevent, int fd, bool remainfd);

/* 处理残留的fd,epoll_wait超时和主循环里面调用 @timeout:是否是超时的时候调用该函数 */
void commevent_remainfd(struct comm_event *commevent, bool timeout);

/* 往结构体struct comm_event里面增加一个fd相关信息  */
bool commevent_add(struct comm_event *commevent, struct comm_tcp *commtcp, struct cbinfo *finishedcb);

/* 从结构体struct comm_event里面删除一个fd相关信息 */
void commevent_del(struct comm_event *commevent, int fd);





#ifdef __cplusplus
	}
#endif 

#endif /* ifndef __COMM_EVENT_H__ */
