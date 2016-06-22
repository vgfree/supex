/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/23.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_DATA_H__
#define __COMM_DATA_H__

#include "comm_structure.h"

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

/* 初始化一个fd的数据结构体struct comm_data */
bool commdata_init(struct connfd_info **connfd, struct comm_event *commevent, struct comm_tcp *commtcp, struct cbinfo *finishedcb);

/* 销毁一个fd的数据结构体struct comm_data*/
void commdata_destroy(struct connfd_info *connfd);

/* 开始打包一个fd的数据 */
bool commdata_package(struct connfd_info *connfd, struct comm_event *commevent, int fd);

/* 开始解析一个fd的数据 */
bool commdata_parse(struct connfd_info *connfd, struct comm_event *commevent, int fd);

bool commdata_send(struct connfd_info *connfd, struct comm_event *commevent, int fd);

bool commdata_recv(struct connfd_info *connfd, struct comm_event *commevent, int fd);

/* 往结构体struct comm_event里面增加一个fd相关信息  */
bool commdata_add(struct comm_event *commevent, struct comm_tcp *commtcp, struct cbinfo *finishedcb);

/* 从结构体struct comm_event里面删除一个fd相关信息 */
void commdata_del(struct comm_event *commevent, int fd);

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __COMM_DATA_H__ */

