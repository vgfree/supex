/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/13.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#include "comm_pipe.h"

#define PIPE_MAX_SIZE ((1 << 8) * 1024)

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

		fcntl(commpipe->rfd, F_SETPIPE_SZ, PIPE_MAX_SIZE);
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

