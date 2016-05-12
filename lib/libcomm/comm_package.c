/*********************************************************************************************/
/************************	Created by 许莉 on 16/05/11.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/

#include "comm_package.h"

bool package_data(struct comm_data *commdata)
{
	bool			flag = false;
	bool			block = false;
	int			packsize = 0; /* 打包之后的数据大小 */
	struct comm_context*	commctx = commdata->commctx;
	struct comm_message*	message = NULL;

	commlock_lock(&commdata->sendlock);
	if (likely(commqueue_pull(&commdata->send_queue, (void*)&message))) {
		if (!commdata->send_queue.writeable) {
			commlock_wake(&commdata->sendlock, &commdata->send_queue.writeable, 1, true);
		}
		flag = true;
	} else {
#if 0		/* 无数据可打包的时候 应该直接退出 */
			commdata->send_queue.readable = 0;
			if (commlock_wait(&commdata->sendlock, &commdata->send_queue.readable, 1, timeout, true)) {
				block = true;
				continue ;
			} else {
				break ;
			}
		if (unlikely(block && commdata->send_queue.nodes == 0)) {
			commdata->send_queue.readable = 0;
		}
#endif
		flag = false;
	}

	commlock_unlock(&commdata->sendlock);

	if (likely(flag)) {
		int size = 0;
		size = mfptp_check_memory(commdata->send_buff.capacity - commdata->send_buff.size, message->package.frames, message->package.dsize);
		if (size > 0) {
			/* 检测到内存不够 则增加内存*/
			if (unlikely(!commcache_expend(&commdata->send_buff, size))) {
				/* 增加内存失败 */
				flag = false;
			} else {
				/* 增加内存成功 */
			}
		}
		if (likely(flag)) {
			mfptp_fill_package(&commdata->packager, message->package.frame_offset, message->package.frame_size, message->package.frames_of_package, message->package.packages);
			packsize = mfptp_package(&commdata->packager, message->content, message->config, message->socket_type);
			if (packsize > 0 && commdata->packager.ms.error == MFPTP_OK) {
				commdata->send_buff.end += packsize;
				//commdata->send_buff.size += packsize;
				log("package successed\n");
			} else {
				flag = false;
				log("package failed\n");
			}
		}
		free_commmsg(message);
	}

	return flag;
}
