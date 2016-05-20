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
	struct comm_list*	list = NULL;
	struct comm_context*	commctx = commdata->commctx;
	struct comm_message*	message = NULL;

	commlock_lock(&commdata->sendlock);
	if (likely(commqueue_pull(&commdata->send_queue, (void*)&message))) {
		flag = true;
	} else if (likely(commlist_pull(&commctx->head, &list))) {
		/* 无数据可打包的时候 应该直接退出 */
		message = (struct comm_message*)get_container_addr(list, COMMMSG_OFFSET);
		if (message->fd == commdata->commtcp.fd) {
			flag = true;
		} else {
			flag = false;
			commdata = (struct comm_data*)commctx->data[message->fd];
		}
	}
	commlock_unlock(&commdata->sendlock);

	if (likely(message)) {
		int size = 0;
		bool pckflag = true;
		size = mfptp_check_memory(commdata->send_cache.capacity - commdata->send_cache.size, message->package.frames, message->package.dsize);
		if (size > 0) {
			/* 检测到内存不够 则增加内存*/
			if (unlikely(!commcache_expend(&commdata->send_cache, size))) {
				/* 增加内存失败 */
				pckflag = false;
			}
		}
		if (likely(pckflag)) {
			mfptp_fill_package(&commdata->packager, message->package.frame_offset, message->package.frame_size, message->package.frames_of_package, message->package.packages);
			packsize = mfptp_package(&commdata->packager, message->content, message->config, message->socket_type);
			if (packsize > 0 && commdata->packager.ms.error == MFPTP_OK) {
				commdata->send_cache.end += packsize;
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
