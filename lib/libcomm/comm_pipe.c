/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_pipe.h"

#define PIPE_READ_MIOU 1024	/* 一次性读取数据的大小 */

bool commpipe_create(struct comm_pipe *commpipe)
{
	assert(commpipe);

	memset(commpipe, 0, sizeof(*commpipe));

	int fda[2] = { 0 };
	if (pipe(fda) == 0) {
		/* 写端口默认阻塞，直到写入数据返回为止 */
		commpipe->wfd = fda[1];

		/* 读端口设置为非阻塞，没有数据就立刻返回 */
		commpipe->rfd = fda[0];
		if (unlikely(!fd_setopt(commpipe->rfd, O_NONBLOCK))) {
			close(commpipe->rfd);
			close(commpipe->wfd);
			return false;
		}

		/* 创建成功 */
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
		loger("destroy commpipe\n");
	}
}

/* @buff为一个数组的首地址， @size为单个数组的大小 @返回值为数组被填充了几个 */
inline int commpipe_read(struct comm_pipe *commpipe, void *buff, int size)
{
	assert(commpipe && commpipe->init);

	int     n = 0;
	int     bytes = 0;

	bytes = read(commpipe->rfd, buff, PIPE_READ_MIOU);
	n = bytes / size;
	commpipe->rcnt += n;
	loger("read one time:%d total read times:%d total write times:%d\n", n, commpipe->rcnt, commpipe->wcnt);
	return n > 0 ? n : -1;
}

/* @buff:待发送数据的首地址 @size:待发送数据的大小 @返回值为发送出去的数据的大小 */
inline int commpipe_write(struct comm_pipe *commpipe, void *buff, int size)
{
	assert(commpipe && commpipe->init);
	int test = size / (sizeof(int));
	commpipe->wcnt += test;
	// loger("commpipe write: %d\n",test);
	// loger("commpipe write total:%d\n", commpipe->wcnt);
	return write(commpipe->wfd, buff, size);
}

