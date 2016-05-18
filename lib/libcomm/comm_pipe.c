/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_pipe.h"

#define PIPE_READ_MIOU	1024	/* 一次性读取数据的大小 */

bool commpipe_init(struct comm_pipe *commpipe) 
{
	assert(commpipe);

	int fda[2] = {0};
	memset(commpipe, 0, sizeof(*commpipe));
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

/* @buff为一个数组的首地址， @size为单个数组的大小 @返回值为数组被填充了几个 */
inline int commpipe_read(struct comm_pipe *commpipe, void *buff, int size) 
{
	assert(commpipe && commpipe->init);

	int bytes = 0;
	if ((bytes = read(commpipe->rfd, buff, PIPE_READ_MIOU)) > 0) {
		return bytes/size;
	} else {
		return bytes;
	}
}

/* @buff:待发送数据的首地址 @size:待发送数据的大小 @返回值为发送出去的数据的大小 */
inline int commpipe_write(struct comm_pipe *commpipe, void *buff, int size)
{
	assert(commpipe && commpipe->init);

	return write(commpipe->wfd, buff, size);
}
