/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_pipe.h"

#define READ_MIOU	1024

bool commpipe_init(struct comm_pipe *commpipe) 
{
	assert(commpipe);
	memset(commpipe, 0, sizeof(*commpipe));
	int fda[2] = {0};
	if (pipe(fda) == 0) {
		/* 创建成功 */
		commpipe->rfd = fda[0];
		commpipe->wfd = fda[1];
		commpipe->init = true;
		return true;
	} else {
		return false;
	}
}

void commpipe_destroy(struct comm_pipe *commpipe) 
{
	if (commpipe && commpipe->init) {
		close(commpipe->rfd);
		close(commpipe->wfd);
		commpipe->init = false;
	}
	return ;
}

/* @buff为一个数组的首地址， @size为单个数组的大小 返回值为数组被填充了几个 */
inline int commpipe_read(struct comm_pipe *commpipe, void *buff, int size) 
{
	assert(commpipe && commpipe->init);
	int bytes = -1;
	bytes = read(commpipe->rfd, buff, READ_MIOU);
	if (bytes > 0) {
		return bytes/size;
	} else {
		return bytes;
	}
}

/* 发送一个数据到管道 */
inline int commpipe_write(struct comm_pipe *commpipe, void *buff, int size)
{
	assert(commpipe && commpipe->init);
	int bytes = -1;
	bytes = write(commpipe->wfd, buff, size);
	return bytes;
}
